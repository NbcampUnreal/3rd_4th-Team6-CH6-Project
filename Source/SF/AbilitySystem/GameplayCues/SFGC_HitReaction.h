// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Burst.h"
#include "SFGC_HitReaction.generated.h"

/**
 * 
 */
UCLASS()
class SF_API USFGC_HitReaction : public UGameplayCueNotify_Burst
{
	GENERATED_BODY()


public:
	virtual void HandleGameplayCue(AActor* Target,
								   EGameplayCueEvent::Type EventType,
								   const FGameplayCueParameters& Parameters) override;
protected:
	UPROPERTY(EditAnywhere, Category="SF|FX")
	TObjectPtr<UNiagaraSystem> LightHitEffect;

	UPROPERTY(EditAnywhere, Category="SF|FX")
	TObjectPtr<UNiagaraSystem> HeavyHitEffect;
	
	UPROPERTY(EditAnywhere, Category="SF|FX|Default")
	TObjectPtr<UNiagaraSystem> DefaultHitEffect;

	UPROPERTY(EditAnywhere, Category="SF|Sound")
	TObjectPtr<USoundBase> LightHitSound;

	UPROPERTY(EditAnywhere, Category="SF|Sound")
	TObjectPtr<USoundBase> HeavyHitSound;
	
	UPROPERTY(EditAnywhere, Category="SF|Sound|Default")
	TObjectPtr<USoundBase> DefaultHitSound;
	
	UPROPERTY(EditAnywhere, Category="SF|Sound")
	float VolumeMultiplier = 1.0f; //볼륨

	UPROPERTY(EditAnywhere, Category="SF|Sound")
	float PitchMultiplier = 1.0f; //피치
	
};
