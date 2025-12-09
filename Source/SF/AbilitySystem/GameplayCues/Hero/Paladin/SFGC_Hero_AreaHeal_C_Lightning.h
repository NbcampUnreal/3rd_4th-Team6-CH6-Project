#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "SFGC_Hero_AreaHeal_C_Lightning.generated.h"

class UParticleSystem;
class USoundBase;

UCLASS()
class SF_API USFGC_Hero_AreaHeal_C_Lightning : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:

	//=====================GameplayCue 진입=====================
	virtual void HandleGameplayCue(
		AActor* Target,
		EGameplayCueEvent::Type EventType,
		const FGameplayCueParameters& Parameters) override;
	//========================================================

protected:

	//=====================파티클=====================
	UPROPERTY(EditAnywhere, Category="SF|FX")
	TObjectPtr<UParticleSystem> LightningEffect1; //기본 파티클

	UPROPERTY(EditAnywhere, Category="SF|FX")
	TObjectPtr<UParticleSystem> LightningEffect2; //추가 파티클(선택)

	UPROPERTY(EditAnywhere, Category="SF|FX")
	FVector ParticleScale = FVector(1.f,1.f,1.f); //크기
	//================================================

	//=====================사운드=====================
	UPROPERTY(EditAnywhere, Category="SF|Sound")
	TObjectPtr<USoundBase> LightningSound1; //사운드1

	UPROPERTY(EditAnywhere, Category="SF|Sound")
	TObjectPtr<USoundBase> LightningSound2; //사운드2

	UPROPERTY(EditAnywhere, Category="SF|Sound")
	float VolumeMultiplier = 1.0f; //볼륨

	UPROPERTY(EditAnywhere, Category="SF|Sound")
	float PitchMultiplier = 1.0f; //피치
	//================================================

	//=====================옵션=====================
	UPROPERTY(EditAnywhere, Category="SF|FX")
	bool bUseTargetFloorIfNoLocation = true; //위치없으면 발밑 사용

	UPROPERTY(EditAnywhere, Category="SF|FX", meta=(EditCondition="bUseTargetFloorIfNoLocation"))
	float FloorTraceUp = 200.f; //위

	UPROPERTY(EditAnywhere, Category="SF|FX", meta=(EditCondition="bUseTargetFloorIfNoLocation"))
	float FloorTraceDown = 400.f; //아래

	FVector ResolveSpawnLocation(AActor* Target,const FGameplayCueParameters& Parameters) const; //발밑 보정
	//================================================
};
//============================================================
