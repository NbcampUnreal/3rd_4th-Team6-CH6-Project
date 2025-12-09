#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "SFGC_SkillPhaseFX.generated.h"

class USFDA_SkillPhaseFX;

UCLASS()
class SF_API USFGC_SkillPhaseFX : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:

	//=====================GameplayCue 진입=============================
	virtual void HandleGameplayCue(AActor* Target,
								   EGameplayCueEvent::Type EventType,
								   const FGameplayCueParameters& Parameters) override;
	//=================================================================

protected:

	//=====================바닥 위치 계산==============================
	FVector GetFloorLocationForActor(AActor* Target) const;
	//=================================================================
};
