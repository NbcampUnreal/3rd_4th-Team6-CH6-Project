#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/GameplayCues/Hero/SFSkillFXTypes.h"
#include "SFAN_SkillFXGameplayCue.generated.h"

class USFDA_SkillPhaseFX;

//================== 스킬 FX GameplayCue 노티파이 ==================
// - 애님 몽타주에서 특정 프레임에 배치
// - 지정된 GameplayCueTag를 실행하면서
//   FX DataAsset + Phase 정보를 함께 전달
//===============================================================
UCLASS()
class SF_API USFAN_SkillFXGameplayCue : public UAnimNotify
{
	GENERATED_BODY()

public:
	//실행할 GameplayCue 태그
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GameplayCue")
	FGameplayTag GameplayCueTag;

	//이 스킬용 FX 데이터 에셋
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GameplayCue")
	TObjectPtr<USFDA_SkillPhaseFX> FXData = nullptr;

	//이번 노티파이가 의미하는 페이즈(CastStart/Loop/Activate)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GameplayCue")
	ESFSkillFXPhase Phase = ESFSkillFXPhase::CastStart;

public:
	//노티파이 이름(에디터 타임라인에 보이는 이름)
	virtual FString GetNotifyName_Implementation() const override
	{
		return TEXT("SkillFXCue");
	}

	//노티파이 실행 시 호출
	virtual void Notify(USkeletalMeshComponent* MeshComp,
						UAnimSequenceBase* Animation,
						const FAnimNotifyEventReference& EventReference) override;
};
