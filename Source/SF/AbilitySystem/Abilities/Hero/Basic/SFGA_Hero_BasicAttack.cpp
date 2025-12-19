#include "SFGA_Hero_BasicAttack.h"

// Engine & Ability System Includes
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayTag.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

// Project Includes
#include "AbilitySystem/GameplayEvent/SFGameplayEventTags.h"
#include "Character/SFCharacterGameplayTags.h"
#include "Input/SFInputGameplayTags.h"
#include "SFBasicAttackData.h"

USFGA_Hero_BasicAttack::USFGA_Hero_BasicAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	
	// 기본값 설정
	ComboWindowTag = SFGameplayTags::Character_State_ComboWindow;
}

void USFGA_Hero_BasicAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// 1. 데이터 에셋 로드 및 검증
	if (AttackDataAsset)
	{
		AttackSteps = AttackDataAsset->AttackSteps;
	}

	if (AttackSteps.Num() == 0)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 2. 이벤트 리스너 등록
	
	// 입력 감지 (InputTag -> GameplayEvent)
	UAbilityTask_WaitGameplayEvent* InputTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, SFGameplayTags::InputTag_Attack);
	if (InputTask)
	{
		InputTask->EventReceived.AddDynamic(this, &ThisClass::OnInputPressedEvent);
		InputTask->ReadyForActivation();
	}

	// 히트 판정 감지 (WeaponActor -> GameplayEvent)
	UAbilityTask_WaitGameplayEvent* HitTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, SFGameplayTags::GameplayEvent_TraceHit);
	if (HitTask)
	{
		HitTask->EventReceived.AddDynamic(this, &ThisClass::OnTraceHitReceived);
		HitTask->ReadyForActivation();
	}

	// 3. 첫 번째 공격 실행
	ExecuteAttackStep(CurrentStepIndex);
}

void USFGA_Hero_BasicAttack::ExecuteAttackStep(int32 StepIndex)
{
	if (!AttackSteps.IsValidIndex(StepIndex))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// 상태 초기화 및 설정
	RemoveStepGameplayTags();
	UpdateRotationToInput();

	const FSFBasicAttackStep& CurrentStep = AttackSteps[StepIndex];
	ApplyStepGameplayTags(CurrentStep.TempAbilityTags);

	// [중요] 매 단계마다 태그 제거(콤보 윈도우 종료) 감지 태스크를 갱신
	UAbilityTask_WaitGameplayTagRemoved* TagRemovedTask = UAbilityTask_WaitGameplayTagRemoved::WaitGameplayTagRemove(this, ComboWindowTag);
	if (TagRemovedTask)
	{
		TagRemovedTask->Removed.AddDynamic(this, &ThisClass::OnComboWindowTagRemoved);
		TagRemovedTask->ReadyForActivation();
	}

	// 차징 로직 (필요 시)
	if (CurrentStep.bIsChargeStep)
	{
		bIsCharging = true;
		ChargeStartTime = GetWorld()->GetTimeSeconds();
	}
	
	// 몽타주 재생 (BlendInTime을 0.1f로 설정하여 부드러운 전이 유도)
	ActiveMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		CurrentStep.Montage,
		1.0f,
		NAME_None,
		true,
		1.0f,
		0.1f // BlendInTime
	);
	
	if (ActiveMontageTask)
	{
		ActiveMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
		ActiveMontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageFinished);
		ActiveMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageFinished);
		ActiveMontageTask->ReadyForActivation();
	}
}

void USFGA_Hero_BasicAttack::OnInputPressedEvent(FGameplayEventData Payload)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC) return;

	// 콤보 윈도우 구간(태그 보유 중)에 입력이 들어왔을 때만 예약
	if (ASC->HasMatchingGameplayTag(ComboWindowTag))
	{
		bInputReserved = true;
	}
}

void USFGA_Hero_BasicAttack::OnComboWindowTagRemoved()
{
	// [핵심 로직] 노티파이 종료 시점에 예약 여부를 확인하고 즉시 다음 단계 실행
	if (bInputReserved)
	{
		bInputReserved = false;
		CurrentStepIndex = (CurrentStepIndex + 1) % AttackSteps.Num();
		
		// 이전 태스크를 명시적으로 정리하여 콜백 충돌 방지
		if (ActiveMontageTask)
		{
			ActiveMontageTask->EndTask();
			ActiveMontageTask = nullptr;
		}
		
		ExecuteAttackStep(CurrentStepIndex);
	}
}

void USFGA_Hero_BasicAttack::OnMontageFinished()
{
	bIsCharging = false;
	RemoveStepGameplayTags();

	// 예약된 입력이 없는 상태에서 몽타주가 자연 종료되면 어빌리티 종료
	// (bInputReserved가 true라면 OnComboWindowTagRemoved에서 이미 처리됨)
	if (!bInputReserved)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void USFGA_Hero_BasicAttack::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (bIsCharging)
	{
		float ChargeDuration = GetWorld()->GetTimeSeconds() - ChargeStartTime;
		bIsCharging = false;

		if (AttackSteps.IsValidIndex(CurrentStepIndex))
		{
			const FSFBasicAttackStep& CurrentStep = AttackSteps[CurrentStepIndex];
			
			// 차징 성공 시 다음 단계(발사 등)로 진행
			if (ChargeDuration >= CurrentStep.MinChargeTime)
			{
				CurrentStepIndex++; 
				ExecuteAttackStep(CurrentStepIndex);
			}
			else
			{
				EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
			}
		}
	}
}

void USFGA_Hero_BasicAttack::OnTraceHitReceived(FGameplayEventData Payload)
{
	// TODO: Payload.TargetData를 활용한 대미지 계산 및 GE 적용
	// ApplyGameplayEffectSpecToTarget(...)
}

void USFGA_Hero_BasicAttack::UpdateRotationToInput()
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character) return;

	// 입력 벡터 확인
	const FVector InputVector = Character->GetLastMovementInputVector();
	if (!InputVector.IsNearlyZero())
	{
		FRotator TargetRotation = InputVector.Rotation();
		TargetRotation.Pitch = 0.f;
		TargetRotation.Roll = 0.f;
		
		Character->SetActorRotation(TargetRotation);
	}
}

void USFGA_Hero_BasicAttack::ApplyStepGameplayTags(const FGameplayTagContainer& Tags)
{
	if (Tags.IsEmpty()) return;

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->AddLooseGameplayTags(Tags);
		AppliedStepTags = Tags;
	}
}

void USFGA_Hero_BasicAttack::RemoveStepGameplayTags()
{
	if (AppliedStepTags.IsEmpty()) return;

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveLooseGameplayTags(AppliedStepTags);
		AppliedStepTags.Reset();
	}
}

void USFGA_Hero_BasicAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	RemoveStepGameplayTags();
	
	// 상태 변수 초기화
	CurrentStepIndex = 0;
	bInputReserved = false;
	bIsCharging = false;
	ActiveMontageTask = nullptr;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}