#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"
#include "SFGC_Hero_AreaHeal_Ground.generated.h"

class UParticleSystem;
class UParticleSystemComponent;
class USoundBase;
class UAudioComponent;
class UNiagaraSystem;
class UNiagaraComponent;

UCLASS()
class SF_API ASFGC_Hero_AreaHeal_Ground : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()

public:
	ASFGC_Hero_AreaHeal_Ground();

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SF|AreaHeal|VFX")
	UParticleSystem* GroundCascade; //캐스케이드 FX

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SF|AreaHeal|VFX")
	UNiagaraSystem* GroundNiagara; //나이아가라 FX

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SF|AreaHeal|Audio")
	USoundBase* GroundSound; //사운드

	UPROPERTY(Transient)
	UParticleSystemComponent* SpawnedCascadeComp; //실행 중 Cascade

	UPROPERTY(Transient)
	UNiagaraComponent* SpawnedNiagaraComp; //실행 중 Niagara

	UPROPERTY(Transient)
	UAudioComponent* SpawnedAudioComp; //실행 중 Audio

public:

	virtual bool OnActive_Implementation(
		AActor* MyTarget,
		const FGameplayCueParameters& Parameters) override; //발동 시

	virtual bool OnRemove_Implementation(
		AActor* MyTarget,
		const FGameplayCueParameters& Parameters) override; //종료 시
};
