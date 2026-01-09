#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "SFAnimNotify_PlayCameraShake.generated.h"

class ULegacyCameraShake;
/**
 * 
 */
UCLASS(meta=(DisplayName="Play Camera Shake"))
class SF_API USFAnimNotify_PlayCameraShake : public UAnimNotify
{
	GENERATED_BODY()

public:
	USFAnimNotify_PlayCameraShake(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void Notify(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<ULegacyCameraShake> CameraShakeClass;

	UPROPERTY(EditAnywhere)
	float PlayScale = 1.f;
};
