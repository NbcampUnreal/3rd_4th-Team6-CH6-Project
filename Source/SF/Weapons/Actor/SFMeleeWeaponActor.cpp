// SFMeleeWeaponActor.cpp
#include "SFMeleeWeaponActor.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystem/GameplayEvent/SFGameplayEventTags.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "AbilitySystem/GameplayEffect/SFGameplayEffectContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SFMeleeWeaponActor)

ASFMeleeWeaponActor::ASFMeleeWeaponActor(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
    RootComponent = StaticMeshComponent;
    StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    StaticMeshComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
    StaticMeshComponent->SetSimulatePhysics(false);

    WeaponCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("WeaponCollision"));
    WeaponCollision->SetupAttachment(StaticMeshComponent);
    WeaponCollision->SetCollisionObjectType(ECC_WorldDynamic);
    WeaponCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
    WeaponCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    WeaponCollision->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
    WeaponCollision->SetGenerateOverlapEvents(true);
    WeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WeaponCollision->SetBoxExtent(FVector(50.f, 10.f, 50.f));
    
    WeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &ASFMeleeWeaponActor::OnWeaponOverlap);

    TraceRefCounts = 0;
}

bool ASFMeleeWeaponActor::CanBeTraced() const
{
    return CurrentWeaponOwner != nullptr;
}

void ASFMeleeWeaponActor::OnTraceStart(AActor* WeaponOwner)
{
    if (!WeaponOwner) return;

    // 1. 카운트 증가
    TraceRefCounts++;
    CurrentWeaponOwner = WeaponOwner;
    
    // 2. 새로운 공격이므로 히트 목록 초기화
    HitActorsThisAttack.Empty();

    if (WeaponCollision)
    {
        // 3. 충돌 활성화
        WeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        WeaponCollision->UpdateOverlaps(); 

        // 4. [핵심 Fix] 이미 칼 안에 들어와 있는 적들을 수동으로 찾아 때립니다.
        //    (연속 공격 시 BeginOverlap이 발생 안 하는 문제를 해결함)
        TArray<UPrimitiveComponent*> OverlappingComps;
        WeaponCollision->GetOverlappingComponents(OverlappingComps);

        for (UPrimitiveComponent* Comp : OverlappingComps)
        {
            if (Comp && Comp->GetOwner())
            {
                ProcessOverlap(Comp->GetOwner(), Comp);
            }
        }
    }
}

void ASFMeleeWeaponActor::OnTraceEnd(AActor* WeaponOwner)
{
    TraceRefCounts--;

    // 콤보 등으로 윈도우가 겹쳐있으면 아직 끄지 않음
    if (TraceRefCounts > 0)
    {
        return;
    }

    if (WeaponCollision)
    {
        WeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    CurrentWeaponOwner = nullptr;
    HitActorsThisAttack.Empty();
    TraceRefCounts = 0;
}

// 기존 OnWeaponOverlap 코드를 분리하여 재사용
void ASFMeleeWeaponActor::ProcessOverlap(AActor* TargetActor, UPrimitiveComponent* TargetComp)
{
    // 유효성 검사
    if (!TargetActor || !CurrentWeaponOwner) return;
    if (TargetActor == this || TargetActor == CurrentWeaponOwner) return;
    
    // 이미 이번 공격에서 때린 적이면 무시
    if (HitActorsThisAttack.Contains(TargetActor)) return;

    // 히트 기록
    HitActorsThisAttack.Add(TargetActor);

    // HitResult 생성
    FHitResult HitInfo;
    HitInfo.HitObjectHandle = FActorInstanceHandle(TargetActor);
    HitInfo.Component = TargetComp;

    // 맞은 부위 판단 (ClosestBone)
    if (USkeletalMeshComponent* SkeletalMesh = Cast<USkeletalMeshComponent>(TargetComp))
    {
        FVector WeaponLocation = WeaponCollision->GetComponentLocation();
        HitInfo.BoneName = SkeletalMesh->FindClosestBone(WeaponLocation);
        HitInfo.ImpactPoint = SkeletalMesh->GetBoneLocation(HitInfo.BoneName);
    }
    else
    {
        HitInfo.BoneName = NAME_None;
        HitInfo.ImpactPoint = TargetActor->GetActorLocation();
    }

    HitInfo.Location = HitInfo.ImpactPoint;
    HitInfo.ImpactNormal = (HitInfo.ImpactPoint - CurrentWeaponOwner->GetActorLocation()).GetSafeNormal();
    HitInfo.bBlockingHit = false;

    // 최종 처리 (GAS 이벤트 전송)
    OnTraced(HitInfo, CurrentWeaponOwner);
}

void ASFMeleeWeaponActor::OnWeaponOverlap( UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // 엔진 이벤트가 발생하면 내부 함수 호출
    ProcessOverlap(OtherActor, OtherComp);
}

void ASFMeleeWeaponActor::OnTraced(const FHitResult& HitInfo, AActor* WeaponOwner)
{
    if (!WeaponOwner) return;

    UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(WeaponOwner);
    if (!OwnerASC) return;
    
    FGameplayEffectContextHandle ContextHandle = OwnerASC->MakeEffectContext();
    FSFGameplayEffectContext* SFContext = static_cast<FSFGameplayEffectContext*>(ContextHandle.Get());

    SFContext->AddHitResult(HitInfo);     
    SFContext->AddSourceObject(this);     
    
    FGameplayEventData EventData;
    EventData.Instigator = WeaponOwner;      
    EventData.Target = HitInfo.GetActor();    
    EventData.ContextHandle = ContextHandle;  
    
    FGameplayAbilityTargetDataHandle TargetDataHandle;
    TargetDataHandle.Add(new FGameplayAbilityTargetData_SingleTargetHit(HitInfo));
    EventData.TargetData = TargetDataHandle;

    OwnerASC->HandleGameplayEvent(SFGameplayTags::GameplayEvent_TraceHit, &EventData);
}