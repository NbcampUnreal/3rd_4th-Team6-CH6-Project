#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"
#include "SFGC_PlayCosmetic_Actor.generated.h"

class UNiagaraComponent;
class UAudioComponent;

UCLASS()
class SF_API ASFGC_PlayCosmetic_Actor : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()

public:
	ASFGC_PlayCosmetic_Actor();

protected:
	
	virtual bool OnActive_Implementation(AActor* MyTarget,const FGameplayCueParameters& Parameters) override;

	
	virtual bool OnRemove_Implementation( AActor* MyTarget, const FGameplayCueParameters& Parameters ) override;

protected:
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> NiagaraComp;

	UPROPERTY()
	TObjectPtr<UAudioComponent> AudioComp;
};
