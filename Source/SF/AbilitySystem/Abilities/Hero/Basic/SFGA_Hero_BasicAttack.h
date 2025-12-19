#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/SFGameplayAbility.h"
#include "SFBasicAttackTypes.h"
#include "SFGA_Hero_BasicAttack.generated.h"

class USFBasicAttackData;
class UAbilityTask_PlayMontageAndWait;

/**
 * [SF] 기본 공격 어빌리티
 * * - 데이터 에셋 기반의 콤보/차징 시스템 지원
 * - AnimNotifyState(ComboWindow) 종료 시점을 이용한 정밀한 콤보 전이
 * - AbilityTask 재사용을 통한 멀티플레이어 동기화 최적화
 */
UCLASS(Abstract)
class SF_API USFGA_Hero_BasicAttack : public USFGameplayAbility
{
	GENERATED_BODY()

public:
	USFGA_Hero_BasicAttack();

	//~ Begin UGameplayAbility Interface
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	//~ End UGameplayAbility Interface

protected:
	/** 현재 인덱스의 공격 단계를 실행합니다. */
	void ExecuteAttackStep(int32 StepIndex);

	/** 현재 입력 방향으로 캐릭터를 회전시킵니다. */
	void UpdateRotationToInput();

	/** 단계별 임시 태그(슈퍼아머 등)를 적용/해제합니다. */
	void ApplyStepGameplayTags(const FGameplayTagContainer& Tags);
	void RemoveStepGameplayTags();

protected: /* Event Handlers */

	/** 입력 키가 눌렸을 때 호출 (콤보 예약 처리) */
	UFUNCTION()
	void OnInputPressedEvent(FGameplayEventData Payload);

	/** 콤보 윈도우(NotifyState) 태그가 제거될 때 호출 (다음 콤보 실행 트리거) */
	UFUNCTION()
	void OnComboWindowTagRemoved();

	/** 몽타주 재생이 완전히 종료되었을 때 호출 */
	UFUNCTION()
	void OnMontageFinished();

	/** 무기 트레이스 히트 이벤트 수신 (대미지 처리 진입점) */
	UFUNCTION()
	void OnTraceHitReceived(FGameplayEventData Payload);

protected: /* Configuration */

	// 직업별 공격 데이터 에셋
	UPROPERTY(EditDefaultsOnly, Category = "SF|Attack")
	TObjectPtr<USFBasicAttackData> AttackDataAsset;

	// 실행 중인 콤보 데이터 캐싱
	UPROPERTY(BlueprintReadOnly, Category = "SF|Attack")
	TArray<FSFBasicAttackStep> AttackSteps;

	// 콤보 입력 허용 구간을 식별하는 태그
	UPROPERTY(EditDefaultsOnly, Category = "SF|Attack")
	FGameplayTag ComboWindowTag;

private: /* State Variables */

	int32 CurrentStepIndex = 0;
	bool bInputReserved = false;
	bool bIsCharging = false;
	float ChargeStartTime = 0.0f;

	// 현재 단계에서 적용된 태그 캐싱
	FGameplayTagContainer AppliedStepTags;

	// 실행 중인 몽타주 태스크 관리
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> ActiveMontageTask;
};