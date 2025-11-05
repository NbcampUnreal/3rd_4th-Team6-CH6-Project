// Copyright 1998-2024 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "WeaponSkillBase.generated.h"

/**
 * '웨폰 스킬 베이스 1번' (설계도)
 * 모든 무기 스킬의 C++ 부모 클래스입니다.
 */
UCLASS(Abstract) 
class SF_API UWeaponSkillBase : public UGameplayAbility 
{
	GENERATED_BODY()

public:
	UWeaponSkillBase();

	/** [핵심 부품 1] 이 스킬이 재생할 애님 몽타주입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> MontageToPlay;

protected:
	/**
	 * [핵심 기능 1] 스킬이 실제로 발동되었을 때 호출됩니다. (메인 엔진)
	 */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/**
	 * [핵심 기능 2] 스킬이 종료될 때 호출됩니다. (뒷정리)
	 */
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	// --- 몽타주 재생 태스크의 콜백 함수들 ('결과 보고') ---

	/** (상황 A: 성공) 몽타주 재생이 성공적으로 완료되었을 때 호출됩니다. */
	UFUNCTION()
	virtual void OnMontageCompleted();

	/** (상황 B: 피격) 몽타주가 외부 요인(예: 피격)에 의해 강제로 중단(Interrupted)되었을 때 호출됩니다. */
	UFUNCTION()
	virtual void OnMontageInterrupted();

	/** (상황 C: 내부 캔슬) 몽타주가 이 어빌리티 내부의 로직(예: EndAbility 호출)에 의해 캔슬(Cancelled)되었을 때 호출됩니다. */
	UFUNCTION()
	virtual void OnMontageCancelled();

protected:
	/**
	 * 몽타주 재생을 담당하는 어빌리티 태스크('일꾼')의 포인터입니다.
	 */
	UPROPERTY()
	TObjectPtr<class UAbilityTask_PlayMontageAndWait> MontageTask;
};