#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SFDA_WaveEffectData.generated.h"

class UNiagaraSystem;

UCLASS()
class SF_API USFDA_WaveEffectData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="VFX")
	TObjectPtr<UNiagaraSystem> NiagaraSystem;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Sound")
	TObjectPtr<USoundBase> Sound;

	// VFX 크기 추가 보정 (1.0 = 판정과 동일, 1.2 = 20% 크게)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="VFX|Scale")
	float ScaleMultiplier = 1.0f;
};
