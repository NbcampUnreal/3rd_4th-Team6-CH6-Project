#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "SF_EffectConfig.generated.h"

class UNiagaraSystem;
class USoundBase;
class USoundAttenuation;

UENUM(BlueprintType)
enum class EEffectSpawnType : uint8
{
	Location,  
	Attached   
};

USTRUCT(BlueprintType)
struct FEffectData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EEffectSpawnType SpawnType = EEffectSpawnType::Location;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UNiagaraSystem> Effect_NiagaraSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sound")
	TSoftObjectPtr<USoundBase> Effect_SoundBase;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sound")
	TSoftObjectPtr<USoundAttenuation> Effect_Attenuation;
};

UCLASS()
class SF_API USF_EffectConfig : public UDataAsset
{
	GENERATED_BODY()
    
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FGameplayTag, FEffectData> EffectDataMap;
};