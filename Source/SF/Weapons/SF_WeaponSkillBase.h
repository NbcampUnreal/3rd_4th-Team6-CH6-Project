// Copyright 1998-2024 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemComponent.h"

#include "SF_WeaponSkillBase.generated.h"

// Forward Declarations
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;
class USF_DamageDataAsset;

UCLASS(Abstract) 
class SF_API UWeaponSkillBase : public UGameplayAbility 
{
	GENERATED_BODY()

public:
	UWeaponSkillBase();

	// --- 1. 애니메이션 설정 ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> MontageToPlay;

	// --- 2. [수정됨] 데미지 설정 ---
	/** [수정됨] 데미지 데이터 에셋입니다. (대미지, 크리티컬, 트레이스 설정 등) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	class USF_DamageDataAsset* DamageDataAsset;


protected:
	// --- 어빌리티 핵심 함수 ---
     
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	// --- 1. 몽타주 콜백 함수 ---
	UFUNCTION()
	virtual void OnMontageCompleted();
	UFUNCTION()
	virtual void OnMontageInterrupted();
	UFUNCTION()
	virtual void OnMontageCancelled();

	// --- 2. 데미지 이벤트 콜백 함수 ---
	/** 몽타주에서 DamageEventTag가 발동되면 호출될 함수입니다. */
	UFUNCTION()
	virtual void OnDamageEventReceived(FGameplayEventData Payload);


protected:
	// --- '일꾼' 태스크 포인터 ---

	/** 1. 몽타주 재생을 담당하는 '일꾼'입니다. */
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	/** 2. 데미지 이벤트를 기다리는 '일꾼'입니다. */
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> DamageEventTask;
};