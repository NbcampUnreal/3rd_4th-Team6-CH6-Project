#include "SFGA_Dragon_MeteoDive.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Character/Enemy/Component/SFDragonMovementComponent.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionMoveToForce.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "DrawDebugHelpers.h"
#include "AbilitySystem/GameplayCues/SFGameplayCueTags.h"
#include "Character/SFCharacterGameplayTags.h"
#include "Character/Enemy/Component/Boss_Dragon/SFDragonGameplayTags.h"
#include "GameplayCueFunctionLibrary.h"
#include "Engine/OverlapResult.h"
#include "Kismet/KismetMathLibrary.h"

USFGA_Dragon_MeteorDive::USFGA_Dragon_MeteorDive()
{
    AbilityTags.AddTag(SFGameplayTags::Ability_Dragon_DiveAttack);
    ActivationOwnedTags.AddTag(SFGameplayTags::Character_State_UsingAbility);
    ActivationOwnedTags.AddTag(SFGameplayTags::Character_State_Invulnerable);
    
    // [Core] 어빌리티 종료 전까지 비행 상태 강제 유지
    ActivationOwnedTags.AddTag(SFGameplayTags::Dragon_Movement_Flying);

    bIsCancelable = false;
}

void USFGA_Dragon_MeteorDive::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (!OwnerCharacter)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    if (USFDragonMovementComponent* MoveComp = Cast<USFDragonMovementComponent>(OwnerCharacter->GetCharacterMovement()))
    {
        if (!MoveComp->IsFlying())
        {
            MoveComp->SetFlyingMode(true);
        }
    }
    
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(RotationTimerHandle, this, &ThisClass::UpdateRotationToTarget, 0.01f, true);
    }

    FGameplayCueParameters Indicator;
    Indicator.Location = TargetLandLocation;
    Indicator.RawMagnitude = ImpactRadius;
    FireGameplayCueWithCosmetic_Actor(SFGameplayTags::GameplayCue_Dragon_Indicator,Indicator);
    
    UAbilityTask_WaitDelay* WaitTask = UAbilityTask_WaitDelay::WaitDelay(this, HoverDuration);
    if (WaitTask)
    {
        WaitTask->OnFinish.AddDynamic(this, &ThisClass::OnHoverTimeFinished);
        WaitTask->ReadyForActivation();
    }
    else
    {
        OnHoverTimeFinished();
    }
}

void USFGA_Dragon_MeteorDive::UpdateRotationToTarget()
{
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (!OwnerCharacter) return;

    FVector MyLoc = OwnerCharacter->GetActorLocation();
    FVector TargetPlaneLoc = TargetLandLocation;
    TargetPlaneLoc.Z = MyLoc.Z; 

    FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(MyLoc, TargetPlaneLoc);
    FRotator CurrentRot = OwnerCharacter->GetActorRotation();
    
    // Yaw만 부드럽게 회전
    FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, 0.01f, 5.0f);
    NewRot.Pitch = 0.0f;
    NewRot.Roll = 0.0f;
    
    OwnerCharacter->SetActorRotation(NewRot);
}

void USFGA_Dragon_MeteorDive::OnHoverTimeFinished()
{
    if (GetAbilitySystemComponentFromActorInfo()->HasMatchingGameplayTag(SFGameplayTags::GameplayCue_Dragon_Indicator))
    {
        GetAbilitySystemComponentFromActorInfo()->RemoveGameplayCue(SFGameplayTags::GameplayCue_Dragon_Indicator);
    }
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(RotationTimerHandle);
    }

    ACharacter* OwnerCharacter = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (OwnerCharacter)
    {
        // 다이브 직전 타겟 방향 정렬
        FVector MyLoc = OwnerCharacter->GetActorLocation();
        FVector TargetPlaneLoc = TargetLandLocation;
        TargetPlaneLoc.Z = MyLoc.Z;

        FRotator FinalLookRot = UKismetMathLibrary::FindLookAtRotation(MyLoc, TargetPlaneLoc);
        FinalLookRot.Pitch = 0.0f; 
        OwnerCharacter->SetActorRotation(FinalLookRot);
    }

    LaunchDiveAttack();
}

void USFGA_Dragon_MeteorDive::LaunchDiveAttack()
{
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (!OwnerCharacter) return;

    FVector StartLocation = OwnerCharacter->GetActorLocation();
    float Distance = FVector::Dist(StartLocation, TargetLandLocation);
    
    const float MinDuration = 0.6f; 
    
    float CalcDuration = (DiveSpeed > 0.f) ? (Distance / DiveSpeed) : 1.0f;
    
    float FinalDuration = FMath::Max(CalcDuration, MinDuration);

    //타겟을 향해 Pich회전 
    FRotator LookAtRotation = (TargetLandLocation - StartLocation).Rotation();
    OwnerCharacter->SetActorRotation(LookAtRotation);
    
    if (DiveLoopMontage)
    {
        OwnerCharacter->PlayAnimMontage(DiveLoopMontage);
    }
    
    UAbilityTask_ApplyRootMotionMoveToForce* DiveTask = UAbilityTask_ApplyRootMotionMoveToForce::ApplyRootMotionMoveToForce(
            this, 
            TEXT("MeteorDive"), 
            TargetLandLocation, 
            FinalDuration, 
            true, 
            EMovementMode::MOVE_Flying, 
            false, 
            nullptr,
            ERootMotionFinishVelocityMode::SetVelocity, 
            FVector::ZeroVector, 
            0.0f
        );

    if (DiveTask)
    {
        DiveTask->OnTimedOutAndDestinationReached.AddDynamic(this, &ThisClass::OnDiveFinished);
        DiveTask->OnTimedOut.AddDynamic(this, &ThisClass::OnDiveFinished);
        DiveTask->ReadyForActivation();
    }
}

void USFGA_Dragon_MeteorDive::OnDiveFinished()
{
    ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (!Character)
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }
    
    if (DiveLoopMontage)
    {
        Character->StopAnimMontage(DiveLoopMontage);
    }
  
    if (USFDragonMovementComponent* MoveComp = Cast<USFDragonMovementComponent>(Character->GetCharacterMovement()))
    {
        MoveComp->SetFlyingMode(false); 
        MoveComp->StopMovementImmediately();
        
        if (GetAbilitySystemComponentFromActorInfo()->HasMatchingGameplayTag(SFGameplayTags::GameplayCue_Dragon_Flying))
        {
            GetAbilitySystemComponentFromActorInfo()->RemoveGameplayCue(SFGameplayTags::GameplayCue_Dragon_Flying);
        }
    }
    
    
    FRotator CurrentRot = Character->GetActorRotation();
    FRotator UprightRot = FRotator(0.0f, CurrentRot.Yaw, 0.0f); 
    
    Character->SetActorRotation(UprightRot);
    if (AController* Controller = Character->GetController())
    {
        Controller->SetControlRotation(UprightRot);
    }
    
    FVector CenterLoc = Character->GetActorLocation(); 
    float HalfHeight = 0.0f;

    if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
    {
        HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
    }

    FVector FeetLoc = CenterLoc - FVector(0.0f, 0.0f, HalfHeight);

    FHitResult GroundHit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Character);
    
    
    bool bHitGround = GetWorld()->LineTraceSingleByChannel(
        GroundHit, 
        CenterLoc, 
        FeetLoc - FVector(0.0f, 0.0f, 2000.0f), 
        ECC_Visibility, 
        Params
    );

    FVector FinalCueLoc = bHitGround ? GroundHit.ImpactPoint : FeetLoc;

    FGameplayCueParameters CueParams;
    CueParams.Location = FinalCueLoc;      
    CueParams.EffectCauser = Character;
    CueParams.Instigator = Character;
    CueParams.RawMagnitude = ImpactRadius; 

    FireGameplayCueWithCosmetic_Static(SFGameplayTags::GameplayCue_Dragon_Land, CueParams);
    
    ApplyImpactDamage(FinalCueLoc);
    
    // Debug
    DrawDebugSphere(GetWorld(), FinalCueLoc, ImpactRadius, 32, FColor::Red, false, 5.0f);

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void USFGA_Dragon_MeteorDive::ApplyImpactDamage(const FVector& ImpactLocation)
{
    UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
    if (!SourceASC) return;

    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetAvatarActorFromActorInfo());
    
    FCollisionObjectQueryParams ObjectQueryParams;
    ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);       
    ObjectQueryParams.AddObjectTypesToQuery(ECC_PhysicsBody); 
    
    float CheckRadius = (ImpactRadius > 0.1f) ? ImpactRadius : 500.0f; 

    bool bHit = GetWorld()->OverlapMultiByObjectType(
        OverlapResults,
        ImpactLocation,
        FQuat::Identity,
        ObjectQueryParams,
        FCollisionShape::MakeSphere(CheckRadius), 
        QueryParams
    );

    if (bHit)
    {
        for (const FOverlapResult& Result : OverlapResults)
        {
            AActor* HitActor = Result.GetActor();
            if (!HitActor) continue;
            
            FCollisionQueryParams LoSParams = QueryParams;
            LoSParams.AddIgnoredActor(HitActor);

            FVector TraceStart = ImpactLocation + FVector(0.0f, 0.0f, 50.0f);
            FVector TraceEnd   = HitActor->GetActorLocation();

            FHitResult LoS;
            bool bIsBlocked = GetWorld()->LineTraceSingleByChannel(LoS, TraceStart, TraceEnd, ECC_Visibility, LoSParams);

            if (!bIsBlocked)
            {
                FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
                
                FHitResult HitResult;
                HitResult.Location = ImpactLocation;
                HitResult.ImpactPoint = HitActor->GetActorLocation();
                HitResult.Normal = (HitActor->GetActorLocation() - ImpactLocation).GetSafeNormal();
                HitResult.HitObjectHandle = FActorInstanceHandle(HitActor);

                Context.AddHitResult(HitResult, true);

                ApplyDamageToTarget(HitActor, Context);
                ApplyKnockBackToTarget(HitActor, HitActor->GetActorLocation());
            }
        }
    }
}

void USFGA_Dragon_MeteorDive::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(RotationTimerHandle);
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}