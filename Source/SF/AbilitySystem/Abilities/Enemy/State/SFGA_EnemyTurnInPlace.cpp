#include "SFGA_EnemyTurnInPlace.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/GameplayEvent/SFGameplayEventTags.h"
#include "AbilitySystem/Task/SFAbilityTask_RotateWithCurve.h"
#include "AI/Controller/Dragon/SFDragonController.h"
#include "AI/Controller/SFTurnInPlaceComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

USFGA_EnemyTurnInPlace::USFGA_EnemyTurnInPlace()
{
	bIsRightTurn = false;
	InitialYaw = 0.f;
	RotateTask = nullptr;

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		FAbilityTriggerData Trigger90R;
		Trigger90R.TriggerTag = SFGameplayTags::GameplayEvent_Turn_90R;
		Trigger90R.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(Trigger90R);

		FAbilityTriggerData Trigger90L;
		Trigger90L.TriggerTag = SFGameplayTags::GameplayEvent_Turn_90L;
		Trigger90L.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(Trigger90L);

		FAbilityTriggerData Trigger180R;
		Trigger180R.TriggerTag = SFGameplayTags::GameplayEvent_Turn_180R;
		Trigger180R.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(Trigger180R);

		FAbilityTriggerData Trigger180L;
		Trigger180L.TriggerTag = SFGameplayTags::GameplayEvent_Turn_180L;
		Trigger180L.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(Trigger180L);
	}
}

void USFGA_EnemyTurnInPlace::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	bIsEnding = false;
	USFGameplayAbility::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (ACharacter* Char = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		if (AController* Controller = Char->GetController())
		{
			if (ASFBaseAIController* BaseAI = Cast<ASFBaseAIController>(Controller))
			{
				BaseAI->SetRotationMode(EAIRotationMode::None);
			}
		}

		if (UCharacterMovementComponent* MC = Char->GetCharacterMovement())
		{
			MC->StopMovementImmediately();
			MC->bOrientRotationToMovement = false;
			MC->bUseControllerDesiredRotation = false;
			MC->RotationRate = FRotator::ZeroRotator;
		}

		Char->bUseControllerRotationYaw = false;
	}

	if (!ValidateTriggerEvent(TriggerEventData))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	TriggerEventTag = TriggerEventData->EventTag;
	ActualTurnYaw = TriggerEventData->EventMagnitude;
	bIsRightTurn = ActualTurnYaw > 0.f;

	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		InitialYaw = Avatar->GetActorRotation().Yaw;
		TargetYaw = FMath::UnwindDegrees(InitialYaw + ActualTurnYaw);
	}

	// 몽타주 실행 및 길이 가져오기
	float MontageDuration = 0.f;
	if (!StartTurnMontage(TriggerEventTag, MontageDuration))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 시간 기반 회전 Task 생성
	RotateTask = USFAbilityTask_RotateWithCurve::CreateRotateWithCurveTask(
		this,
		FName("RotateWithCurve"),
		MontageDuration,
		TargetYaw,
		ActualTurnYaw
	);

	if (RotateTask)
	{
		RotateTask->OnCompleted.AddDynamic(this, &USFGA_EnemyTurnInPlace::OnRotationTaskCompleted);
		RotateTask->OnCancelled.AddDynamic(this, &USFGA_EnemyTurnInPlace::OnRotationTaskCancelled);
		RotateTask->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void USFGA_EnemyTurnInPlace::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (bIsEnding) return;
	bIsEnding = true;

	if (RotateTask)
	{
		RotateTask->EndTask();
		RotateTask = nullptr;
	}
	
	bIsRightTurn = false;
	InitialYaw = 0.f;
	
	NotifyTurnFinished();

	USFGameplayAbility::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void USFGA_EnemyTurnInPlace::OnRotationTaskCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void USFGA_EnemyTurnInPlace::OnRotationTaskCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

bool USFGA_EnemyTurnInPlace::StartTurnMontage(const FGameplayTag& EventTag, float& OutDuration)
{
	UAnimMontage* const* MontagePtr = TurnMontageMap.Find(EventTag);
	if (!MontagePtr || !*MontagePtr)
	{
		return false;
	}

	UAnimMontage* Montage = *MontagePtr;

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		float PlayedDuration = ASC->PlayMontage(this, CurrentActivationInfo, Montage, MontagePlayRate);
		
		if (PlayedDuration > 0.f)
		{
			// 몽타주 실제 재생 길이 계산 (PlayRate 적용)
			OutDuration = Montage->GetPlayLength() / MontagePlayRate;
			return true;
		}
	}

	return false;
}

bool USFGA_EnemyTurnInPlace::ValidateTriggerEvent(const FGameplayEventData* TriggerEventData)
{
	if (!TriggerEventData || !TriggerEventData->EventTag.IsValid())
	{
		return false;
	}
	return true;
}

void USFGA_EnemyTurnInPlace::NotifyTurnFinished()
{
	if (ACharacter* Char = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		if (ASFDragonController* DragonAI = Cast<ASFDragonController>(Char->GetController()))
		{
			if (USFTurnInPlaceComponent* TurnComp = DragonAI->GetTurnInPlaceComponent())
			{
				TurnComp->OnTurnFinished();
			}
		}
	}
}