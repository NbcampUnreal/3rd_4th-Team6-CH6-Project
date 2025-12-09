#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"
#include "SFGC_Hero_NSBuffAura.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class USoundBase;
class UAudioComponent;

UCLASS()
class SF_API ASFGC_Hero_NSBuffAura : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()

public:
	ASFGC_Hero_NSBuffAura(); //생성자

protected:

	//=====================VFX=====================
	UPROPERTY(EditAnywhere, Category="Aura|VFX")
	UNiagaraSystem* AuraNiagaraFX; //나이아가라 FX

	UPROPERTY(Transient)
	UNiagaraComponent* AuraComponent; //생성된 Aura Niagara
	//================================================

	//=====================Loop Sound=====================
	UPROPERTY(EditAnywhere, Category="Aura|Sound")
	USoundBase* AuraLoopSound; //루프 사운드

	UPROPERTY(Transient)
	UAudioComponent* AuraAudioComp; //생성된 오디오컴포넌트
	//================================================

	//=====================Transform=====================
	UPROPERTY(EditAnywhere, Category="Aura|Transform")
	FVector AuraLocationOffset = FVector(0.f,0.f,-90.f); //위치 오프셋
	//================================================

public:

	virtual bool OnActive_Implementation(
		AActor* Target,
		const FGameplayCueParameters& Parameters) override; //처음 시작

	virtual bool WhileActive_Implementation(
		AActor* Target,
		const FGameplayCueParameters& Parameters) override; //지속 유지

	virtual bool OnRemove_Implementation(
		AActor* Target,
		const FGameplayCueParameters& Parameters) override; //종료
};
//============================================================
