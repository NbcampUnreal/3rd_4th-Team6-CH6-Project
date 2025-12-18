// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Burst.h"
#include "SFGC_WaveEffect.generated.h"

/**
 * 
 */
UCLASS()
class SF_API USFGC_WaveEffect : public UGameplayCueNotify_Burst
{
	GENERATED_BODY()

protected:
	virtual bool OnExecute_Implementation(AActor* Target, const FGameplayCueParameters& Parameters) const override;

	UPROPERTY(EditDefaultsOnly, Category="SF|VFX")
	TObjectPtr<UNiagaraSystem> DefaultNiagaraSystem;

	UPROPERTY(EditDefaultsOnly, Category="SF|SFX")
	TObjectPtr<USoundBase> DefaultSound;;

	UPROPERTY(EditDefaultsOnly, Category="SF|VFX")
	FVector BaseScale = FVector(1.f);

	// RawMagnitude 기준값 (이 값일 때 BaseScale)
	UPROPERTY(EditDefaultsOnly, Category="SF|VFX")
	float MagnitudeBase = 500.f;

	UPROPERTY(EditDefaultsOnly, Category="SF|SFX")
	float VolumeMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category="SF|SFX")
	float PitchMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category="SF|VFX")
	bool bSnapToFloor = true;
};
