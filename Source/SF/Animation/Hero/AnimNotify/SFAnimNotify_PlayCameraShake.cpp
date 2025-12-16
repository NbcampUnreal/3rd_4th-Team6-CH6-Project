#include "SFAnimNotify_PlayCameraShake.h"

#include "Character/Hero/SFHero.h"
#include "Player/SFPlayerController.h"
#include "LegacyCameraShake.h"

USFAnimNotify_PlayCameraShake::USFAnimNotify_PlayCameraShake(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	bShouldFireInEditor = false;
#endif
	bIsNativeBranchingPoint = true;
}

void USFAnimNotify_PlayCameraShake::Notify(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComponent, Animation, EventReference);

	ASFHero* SFHero = Cast<ASFHero>(MeshComponent->GetOwner());
	if (!SFHero)
	{
		return;
	}
	
	ASFPlayerController* SFPlayerController = SFHero->GetSFPlayerController();
	if (!SFPlayerController)
	{
		return;
	}
	
	APlayerCameraManager* PlayerCameraManager = SFPlayerController->PlayerCameraManager;
	if (PlayerCameraManager == nullptr)
	{
		return;
	}
	
	ULegacyCameraShake::StartLegacyCameraShake(PlayerCameraManager, CameraShakeClass, PlayScale);
}
