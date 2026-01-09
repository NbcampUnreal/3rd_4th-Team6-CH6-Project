#include "SFGA_Hero_HitReact.h"

#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"
#include "Abilities/Tasks/AbilityTask_NetworkSyncPoint.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/SFAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/SFGameplayAbilityTags.h"
#include "AbilitySystem/GameplayEvent/SFGameplayEventTags.h"
#include "Animation/SFAnimationGameplayTags.h"
#include "Character/SFCharacterGameplayTags.h"
#include "Player/SFPlayerController.h"

USFGA_Hero_HitReact::USFGA_Hero_HitReact(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActivationPolicy = ESFAbilityActivationPolicy::Manual;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	bServerRespectsRemoteAbilityCancellation = true;
	bRetriggerInstancedAbility = true;

	AbilityTags.AddTag(SFGameplayTags::Ability_Hero_HitReact);
	ActivationOwnedTags.AddTag(SFGameplayTags::Character_State_Hit);
	ActivationBlockedTags.AddTag(SFGameplayTags::Character_State_SuperArmor);

	HitReactMontageTag = SFGameplayTags::Montage_State_HitReact;

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = SFGameplayTags::GameplayEvent_HitReaction;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

void USFGA_Hero_HitReact::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (TriggerEventData == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	CachedInstigator = const_cast<AActor*>(TriggerEventData->Instigator.Get());

	if (USFAbilitySystemComponent* ASC = GetSFAbilitySystemComponentFromActorInfo())
	{
		ASC->CancelActiveAbilities(nullptr, nullptr, this);
	}

	// 피격 각도 계산 및 몽타주 캐싱
	const float HitAngle = CalculateHitAngle(TriggerEventData);
	CachedMontageData = SelectHitReactMontage(HitAngle);

	if (!CachedMontageData.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (UAbilityTask_NetworkSyncPoint* NetSyncTask = UAbilityTask_NetworkSyncPoint::WaitNetSync(this, EAbilityTaskNetSyncType::OnlyServerWait))
	{
		NetSyncTask->OnSync.AddDynamic(this, &ThisClass::OnNetSync);
		NetSyncTask->ReadyForActivation();
	}

	if (IsLocallyControlled() && HitReactCameraShakeClass)
	{
		if (ASFPlayerController* SFPC = GetSFPlayerControllerFromActorInfo())
		{
			if (APlayerCameraManager* CameraManager = SFPC->PlayerCameraManager)
			{
				CameraManager->StartCameraShake(HitReactCameraShakeClass);
			}
		}
	}
}

void USFGA_Hero_HitReact::OnNetSync()
{
	DisablePlayerInput();
	
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	if (bApplyMicroKnockback)
	{
		FVector KnockbackDirection;
        
		if (CachedInstigator.IsValid())
		{
			KnockbackDirection = (AvatarActor->GetActorLocation() - CachedInstigator->GetActorLocation()).GetSafeNormal();
		}
		else
		{
			KnockbackDirection = -AvatarActor->GetActorForwardVector();
		}
        
		KnockbackDirection.Z = 0.f;
        
		if (UAbilityTask_ApplyRootMotionConstantForce* MicroKnockbackTask = UAbilityTask_ApplyRootMotionConstantForce::ApplyRootMotionConstantForce(
				this,
				TEXT("MicroKnockback"),
				KnockbackDirection,
				MicroKnockbackStrength,
				MicroKnockbackDuration,
				false,
				MicroKnockbackStrengthCurve,
				ERootMotionFinishVelocityMode::ClampVelocity,
				FVector::ZeroVector,
				50.f,
				false))
		{
			MicroKnockbackTask->ReadyForActivation();
		}
	}
	
	if (!CachedMontageData.IsValid())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	
	if (UAbilityTask_PlayMontageAndWait* HitReactMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("HitReactMontage"),  CachedMontageData.Montage, CachedMontageData.PlayRate, CachedMontageData.StartSection, true))
	{
		HitReactMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
		HitReactMontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageFinished);
		HitReactMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageFinished);
		HitReactMontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageFinished);
		HitReactMontageTask->ReadyForActivation();
	}
	else
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}

float USFGA_Hero_HitReact::CalculateHitAngle(const FGameplayEventData* EventData) const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		return 0.f;
	}

	FVector AttackerLocation = FVector::ZeroVector;
	bool bHasValidLocation = false;

	// Instigator 위치 (캡슐 히트에 적합)
	if (const AActor* InstigatorActor = EventData->Instigator.Get())
	{
		AttackerLocation = InstigatorActor->GetActorLocation();
		bHasValidLocation = true;
	}
	else if (!EventData->ContextHandle.GetOrigin().IsZero())
	{
		AttackerLocation = EventData->ContextHandle.GetOrigin();
		bHasValidLocation = true;
	}
	// HitResult ImpactPoint (폴백)
	else if (const FHitResult* HitResult = EventData->ContextHandle.GetHitResult())
	{
		AttackerLocation = HitResult->ImpactPoint;
		bHasValidLocation = true;
	}

	if (!bHasValidLocation)
	{
		return 0.f; // 정면 피격으로 처리
	}

	// 각도 계산
	const FVector CharacterLocation = AvatarActor->GetActorLocation();
	const FVector CharacterForward = AvatarActor->GetActorForwardVector();
	const FRotator FacingRotator = CharacterForward.Rotation();
	const FRotator ToAttackerRotator = (AttackerLocation - CharacterLocation).GetSafeNormal().Rotation();
	const FRotator DeltaRotator = (ToAttackerRotator - FacingRotator).GetNormalized();
	return DeltaRotator.Yaw;
}

FSFMontagePlayData USFGA_Hero_HitReact::SelectHitReactMontage(float HitAngle) const
{
	const USFHeroAnimationData* AnimData = GetHeroAnimationData();
	if (!AnimData)
	{
		return FSFMontagePlayData();
	}

	const FSFDirectionalMontageData* DirData = AnimData->DirectionalMontages.Find(HitReactMontageTag);
	if (!DirData)
	{
		return FSFMontagePlayData();
	}

	const float YawAbs = FMath::Abs(HitAngle);
    
	// ±60도/120도 기준 방향 판정
	// 정면: |Yaw| < 60 (120도 범위)
	// 후면: |Yaw| > 120 (120도 범위)
	// 좌측: -120 ~ -60 (60도 범위)
	// 우측: 60 ~ 120 (60도 범위)
    
	if (YawAbs < FrontAngleThreshold)
	{
		if (DirData->Front.IsValid())
		{
			return DirData->Front.GetRandom();
		}
	}
	else if (YawAbs > BackAngleThreshold)
	{
		if (DirData->Back.IsValid())
		{
			return DirData->Back.GetRandom();
		}
	}
	else if (HitAngle < 0.f)
	{
		if (DirData->Left.IsValid())
		{
			return DirData->Left.GetRandom();
		}
	}
	else
	{
		if (DirData->Right.IsValid())
		{
			return DirData->Right.GetRandom();
		}
	}

	// Fallback: 정면 몽타주
	if (DirData->Front.IsValid())
	{
		return DirData->Front.GetRandom();
	}

	return FSFMontagePlayData();
}

void USFGA_Hero_HitReact::OnMontageFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void USFGA_Hero_HitReact::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	CachedMontageData = FSFMontagePlayData();
	RestorePlayerInput();
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

