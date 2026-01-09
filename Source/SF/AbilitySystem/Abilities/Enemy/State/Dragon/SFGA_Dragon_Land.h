// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/SFGameplayAbility.h"
#include "SFGA_Dragon_Land.generated.h"

/**
 * 
 */
UCLASS()
class SF_API USFGA_Dragon_Land : public USFGameplayAbility
{
	GENERATED_BODY()

public:
	USFGA_Dragon_Land();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	

protected:
	UFUNCTION()
	void OnLandedCallback(const FHitResult& Hit);

	UFUNCTION()
	void OnMontageFinished();

protected:
	UPROPERTY(EditAnywhere, Category = "Land")
	TObjectPtr<UAnimMontage> LandMontage;
protected:
	// 낙하 속도 
	UPROPERTY(EditAnywhere, Category = "Land")
	float LandingDownwardSpeed = 4000.0f; 

	// 착륙 시 앞으로 가는 관성을 남길지 여부
	UPROPERTY(EditAnywhere, Category = "Land")
	bool bStopHorizontalMovement = true;
};
