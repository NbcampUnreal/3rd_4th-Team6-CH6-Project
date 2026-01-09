#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SFDA_BuffAuraEffectData.generated.h"

class UNiagaraSystem;

UCLASS()
class SF_API USFDA_BuffAuraEffectData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="VFX")
	TObjectPtr<UNiagaraSystem> NiagaraSystem;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Sound")
	TObjectPtr<USoundBase> LoopSound;
};
