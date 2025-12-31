// Fill out your copyright notice in the Description page of Project Settings.

#include "SFDragonCombatComponent.h"
#include "AbilitySystemGlobals.h"
#include "AIController.h"
#include "AbilitySystem/SFAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/Enemy/SFPrimarySet_Enemy.h"
#include "AbilitySystem/Attributes/SFPrimarySet.h"
#include "Character/SFCharacterBase.h"
#include "Character/SFCharacterGameplayTags.h"
#include "Interface/SFEnemyAbilityInterface.h"

USFDragonCombatComponent::USFDragonCombatComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryComponentTick.bCanEverTick = false;
}

void USFDragonCombatComponent::InitializeCombatComponent()
{
    
    Super::InitializeCombatComponent();

    if (!CachedASC) return;

    
    const USFPrimarySet_Enemy* PrimarySet = CachedASC->GetSet<USFPrimarySet_Enemy>();
    if (PrimarySet)
    {
        USFPrimarySet_Enemy* Set = const_cast<USFPrimarySet_Enemy*>(PrimarySet);
        Set->OnTakeDamageDelegate.RemoveDynamic(this, &ThisClass::AddThreat);
        Set->OnTakeDamageDelegate.AddDynamic(this, &ThisClass::AddThreat);
    }

    
    StartSpatialUpdateTimer();
    StartStateMonitorTimer();
    StartThreatUpdateTimer();
}

void USFDragonCombatComponent::AddThreat(float ThreatValue, AActor* Actor)
{
	if (!Actor)
		return;

	if (ThreatMap.Contains(Actor))
	{
		ThreatMap[Actor] += ThreatValue;
	}
	else
	{
		ThreatMap.Add(Actor, ThreatValue);
	}
}

void USFDragonCombatComponent::CleanupThreatMap()
{
	for (auto It = ThreatMap.CreateIterator(); It; ++It)
	{
		AActor* Key = It.Key();
		if (!IsValidTarget(Key))
		{
			It.RemoveCurrent();
		}
	}
}

AActor* USFDragonCombatComponent::GetHighestThreatActor()
{
    if (ThreatMap.Num() == 0) return nullptr;

    // 1. 반드시 nullptr와 0.f로 시작하여 새로 선출합니다.
    AActor* HighestThreatActor = nullptr;
    float HighestValue = -1.f; 

    for (auto& ThreatPair : ThreatMap)
    {
        if (!IsValidTarget(ThreatPair.Key)) continue;

        if (ThreatPair.Value > HighestValue)
        {
            HighestValue = ThreatPair.Value;
            HighestThreatActor = ThreatPair.Key;
        }
    }

    return HighestThreatActor;
}

void USFDragonCombatComponent::EvaluateTarget()
{
    CleanupThreatMap();
    AActor* NewTarget = GetHighestThreatActor();

    // 2. 로그 로직 수정: 찾았을 때와 못 찾았을 때를 명확히 구분
    if (NewTarget)
    {
        // 타겟을 찾은 경우
        if (GetCurrentTarget() != NewTarget)
        {
            UpdateTargetActor(NewTarget);
            UE_LOG(LogTemp, Log, TEXT("[Dragon] New Target Locked: %s"), *NewTarget->GetName());
        }

        CurrentTargetState = EBossTargetState::Locked;
        LastValidTargetTime = GetWorld()->GetTimeSeconds();
        UpdateSpatialData();
        return; // 타겟을 찾았으므로 여기서 종료
    }
    
    // 3. 타겟을 못 찾았을 때만 경고 로그 출력
    if (ThreatMap.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Dragon] ThreatMap has entries, but no Valid Target found (Check IsValidTarget)"));
    }

    // 4. 타겟 상실 시 Grace(유예) 로직 시작
    if (GetCurrentTarget() && CurrentTargetState == EBossTargetState::Locked)
    {
        if (ShouldForceReleaseTarget(GetCurrentTarget()))
        {
            UpdateTargetActor(nullptr);
            CurrentTargetState = EBossTargetState::None;
            return;
        }

        CurrentTargetState = EBossTargetState::Grace;
        UE_LOG(LogTemp, Log, TEXT("[Dragon] Target lost, entering Grace period."));
        return;
    }

    // 5. Grace 기간 만료 체크
	if (CurrentTargetState == EBossTargetState::Grace)
	{
		float CurrentTime = GetWorld()->GetTimeSeconds();
		if (CurrentTime - LastValidTargetTime >= TargetGraceDuration)
		{
			UpdateTargetActor(nullptr);
			CurrentTargetState = EBossTargetState::None;

			CurrentZone = EBossAttackZone::None;
			CachedDistance = 0.f;
			CachedAngle = 0.f;

			UE_LOG(LogTemp, Log, TEXT("[Dragon] Grace expired → Target cleared"));
		}
	}

}

bool USFDragonCombatComponent::SelectAbility(const FEnemyAbilitySelectContext& Context, const FGameplayTagContainer& SearchTags, FGameplayTag& OutSelectedTag)
{
    
	FBossEnemyAbilitySelectContext DragonContext;

	DragonContext.Self = Context.Self;
	DragonContext.Target = Context.Target;
	DragonContext.DistanceToTarget = CachedDistance;
	DragonContext.AngleToTarget = CachedAngle;

	DragonContext.PlayerHealthPercentage = GetPlayerHealthPercent();
	DragonContext.Zone = CurrentZone;
	
	OutSelectedTag = FGameplayTag();

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(DragonContext.Self);
	if (!ASC || SearchTags.IsEmpty())
	{
		return false;
	}

	// [최적화] Spatial Data는 이미 CachedDistance, CachedAngle로 전달되었으므로 중복 계산 제거
	FEnemyAbilitySelectContext ContextWithSpatialData = DragonContext;

	// 후보군과 가중치를 저장할 배열 선언
	TArray<FGameplayTag> Candidates;
	TArray<float> Weights;
	float TotalWeight = 0.f;

	for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		UGameplayAbility* Ability = Spec.Ability;
		if (!Ability)
		{
			continue;
		}

		FGameplayTagContainer AllTags;
		AllTags.AppendTags(Ability->AbilityTags);
		AllTags.AppendTags(Ability->GetAssetTags());

		// Ability가 SearchTags 중 어떤 태그라도 포함하는가?
		if (!AllTags.HasAny(SearchTags))
		{
			continue;
		}

		const FGameplayAbilityActorInfo* ActorInfo = ASC->AbilityActorInfo.Get();

		// 쿨타임 체크
		if (!Ability->CheckCooldown(Spec.Handle, ActorInfo))
		{
			continue;
		}

		ISFEnemyAbilityInterface* AIInterface = Cast<ISFEnemyAbilityInterface>(Ability);
		if (!AIInterface)
		{
			continue;
		}

		// Context에 Spec 전달
		FEnemyAbilitySelectContext ContextWithSpec = ContextWithSpatialData;
		ContextWithSpec.AbilitySpec = &Spec;

		float Score = AIInterface->CalcAIScore(ContextWithSpec);

		// [최적화] 로그 제거 (성능 향상)
		// UE_LOG(LogTemp, Warning, TEXT("    Score: %.2f"), Score);

		// 점수가 0보다 클 때만 후보에 등록
		if (Score > 0.f)
		{
			// SearchTags 와 정확히 매칭되는 태그만 추출
			FGameplayTag UniqueTag;
			for (const FGameplayTag& Tag : AllTags)
			{
				if (SearchTags.HasTagExact(Tag))
				{
					UniqueTag = Tag;
					break;
				}
			}

			//그래도 못찾으면 그냥 AbilityTags 내 첫 태그 사용
			if (!UniqueTag.IsValid())
			{
				if (Ability->AbilityTags.Num() > 0)
				{
					UniqueTag = Ability->AbilityTags.First();
				}
				else if (Ability->GetAssetTags().Num() > 0)
				{
					UniqueTag = Ability->GetAssetTags().First();
				}
			}

			// 후보 등록 - 이전 공격 제외 로직
			if (UniqueTag.IsValid())
			{
				// 이전에 사용한 공격이면 스킵 (후보가 하나도 없을 때는 허용)
				if (UniqueTag == LastSelectedAbilityTag && Candidates.Num() > 0)
				{
					continue;
				}

				Candidates.Add(UniqueTag);
				Weights.Add(Score);
				TotalWeight += Score;
			}
		}
	}

	if (Candidates.Num() == 0)
	{
		return false;
	}

	// 가중치 랜덤 선택 (Weighted Random)
	float RandomValue = FMath::FRandRange(0.f, TotalWeight);

	for (int32 i = 0; i < Candidates.Num(); ++i)
	{
		if (RandomValue <= Weights[i])
		{
			OutSelectedTag = Candidates[i];
			LastSelectedAbilityTag = OutSelectedTag; // 선택한 공격 기록
			return true;
		}
		RandomValue -= Weights[i];
	}

	// 혹시라도 루프를 빠져나오면 마지막 후보 선택
	OutSelectedTag = Candidates.Last();
	LastSelectedAbilityTag = OutSelectedTag; // 선택한 공격 기록
	return true;
}
	
// 공간 정보 갱신 
void USFDragonCombatComponent::UpdateSpatialData()
{
	if (!IsValid(CurrentTarget))
	{
		CurrentZone = EBossAttackZone::None;
		CachedDistance = 0.f;
		CachedAngle = 0.f;
		return;
	}

	AAIController* AIC = GetController<AAIController>();
	if (!AIC || !AIC->GetPawn())
		return;

	APawn* Dragon = AIC->GetPawn();
	
	CachedDistance = FVector::Dist(
		Dragon->GetActorLocation(),
		CurrentTarget->GetActorLocation()
	);
	
	FVector ToTarget = CurrentTarget->GetActorLocation() - Dragon->GetActorLocation();
	ToTarget.Z = 0.f;
	ToTarget.Normalize();

	FVector Forward = Dragon->GetActorForwardVector();
	Forward.Z = 0.f;
	Forward.Normalize();

	CachedAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Forward, ToTarget)));
	
	if (CachedDistance <= MeleeRange)
	{
		CurrentZone = EBossAttackZone::Melee;
	}
	else if (CachedDistance <= MidRange)
	{
		CurrentZone = EBossAttackZone::Mid;
	}
	else if (CachedDistance <= LongRange)
	{
		CurrentZone = EBossAttackZone::Long;
	}
	else
	{
		CurrentZone = EBossAttackZone::OutOfRange;
	}
}

void USFDragonCombatComponent::MonitorTargetState()
{
	if (!IsValid(CurrentTarget))
	{

		PlayerHealthPercent = 1.0f;
		return;
	}

	ASFCharacterBase* Player = Cast<ASFCharacterBase>(CurrentTarget);
	if (!Player)
		return;

	USFAbilitySystemComponent* PlayerASC = Player->GetSFAbilitySystemComponent();
	if (!PlayerASC)
		return;
	
	if (const USFPrimarySet* PrimarySet = PlayerASC->GetSet<USFPrimarySet>())
	{
		PlayerHealthPercent = PrimarySet->GetHealth() / FMath::Max(PrimarySet->GetMaxHealth(), 1.f);
	}
	

}

void USFDragonCombatComponent::StartSpatialUpdateTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			SpatialUpdateTimerHandle,
			this,
			&USFDragonCombatComponent::UpdateSpatialData,
			SpatialUpdateInterval,
			true  
		);
	}
}

void USFDragonCombatComponent::StopSpatialUpdateTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SpatialUpdateTimerHandle);
	}
}

void USFDragonCombatComponent::StartStateMonitorTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			StateMonitorTimerHandle,
			this,
			&USFDragonCombatComponent::MonitorTargetState,
			StateMonitorInterval,
			true  
		);
	}
}

void USFDragonCombatComponent::StopStateMonitorTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StateMonitorTimerHandle);
	}
}

void USFDragonCombatComponent::StartThreatUpdateTimer()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            ThreatUpdateTimerHandle,
            this,
            &USFDragonCombatComponent::EvaluateTarget,
            ThreatUpdateInterval,
            true
        );
    }
}

void USFDragonCombatComponent::StopThreatUpdateTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ThreatUpdateTimerHandle);
	}
}

bool USFDragonCombatComponent::IsValidTarget(AActor* Target) const
{
	if (!IsValid(Target))
	{
		return false;
	}

	ASFCharacterBase* SFCharacter = Cast<ASFCharacterBase>(Target);
	if (!SFCharacter)
	{
		return false;
	}

	USFAbilitySystemComponent* ASC = SFCharacter->GetSFAbilitySystemComponent();
	if (ASC && ASC->HasMatchingGameplayTag(SFGameplayTags::Character_State_Dead))
	{
		return false;
	}

	return true;
}

bool USFDragonCombatComponent::ShouldForceReleaseTarget(AActor* Target) const
{
	if (!Target)
		return true;

	if (!Target->HasActorBegunPlay())
		return true;

	if (Target->IsPendingKillPending())
		return true;

	// 거리 체크
	if (AActor* Owner = GetOwner())
	{
		float Distance = FVector::Dist(Owner->GetActorLocation(), Target->GetActorLocation());
		if (Distance > MaxCombatRange)
			return true;
	}

	// Dead 체크
	ASFCharacterBase* SFCharacter = Cast<ASFCharacterBase>(Target);
	if (SFCharacter)
	{
		USFAbilitySystemComponent* ASC = SFCharacter->GetSFAbilitySystemComponent();
		if (ASC && ASC->HasMatchingGameplayTag(SFGameplayTags::Character_State_Dead))
			return true;
	}

	return false;
}

