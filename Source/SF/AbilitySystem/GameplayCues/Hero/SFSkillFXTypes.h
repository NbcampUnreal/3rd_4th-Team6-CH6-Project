#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NiagaraSystem.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "SFSkillFXTypes.generated.h"

//=====================스킬 FX 페이즈=====================
UENUM(BlueprintType)
enum class ESFSkillFXPhase : uint8
{
	CastStart  UMETA(DisplayName="Cast Start"), //캐스팅 시작
	CastLoop   UMETA(DisplayName="Cast Loop"), //캐스팅 유지 루프
	Activate   UMETA(DisplayName="Activate") //실제 스킬 발동 타이밍
};
//=======================================================


//=====================페이즈별 FX 세트=====================
USTRUCT(BlueprintType)
struct FSFSkillPhaseFX
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FX")
	TObjectPtr<UNiagaraSystem> NiagaraSystem = nullptr; //나이아가라 FX

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FX")
	TObjectPtr<UParticleSystem> CascadeSystem = nullptr; //캐스케이드 FX

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FX")
	TObjectPtr<USoundBase> Sound = nullptr; //사운드

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FX|Control")
	FVector FXScale = FVector(1.f,1.f,1.f); //이펙트 크기

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FX|Control")
	float SoundVolume = 1.f; //사운드 볼륨
};
//========================================================


//=====================스킬 FX 데이터 에셋=====================
UCLASS(BlueprintType)
class SF_API USFDA_SkillPhaseFX : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FX")
	FSFSkillPhaseFX CastStartFX; //캐스팅 시작 FX

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FX")
	FSFSkillPhaseFX CastLoopFX; //캐스팅 루프(중간) FX

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FX")
	FSFSkillPhaseFX ActivateFX; //스킬 발동 FX

public:

	//=====================페이즈 FX 조회=====================
	UFUNCTION(BlueprintCallable, Category="FX")
	const FSFSkillPhaseFX& GetFXForPhase(ESFSkillFXPhase Phase) const
	{
		switch (Phase)
		{
		case ESFSkillFXPhase::CastStart: return CastStartFX; //Start
		case ESFSkillFXPhase::CastLoop: return CastLoopFX; //Loop
		default: return ActivateFX; //Activate
		}
	}
	//=======================================================
};
//=========================================================
