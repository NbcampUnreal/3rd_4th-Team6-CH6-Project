#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "SFAnimNotifyState_OverlayEffect.generated.h"

UENUM(BlueprintType)
enum class ESFOverlayTargetType : uint8
{
	None,
	Equipment,
	Character,
	All,
};

USTRUCT()
struct FSFOverlayEffectProgressInfo
{
	GENERATED_BODY()

public:
	UPROPERTY()
	float ElapsedTime = 0.f;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> OverlayMaterialInstance;
	
	UPROPERTY()
	TArray<TWeakObjectPtr<UMeshComponent>> MeshComponents;
};

UCLASS(Meta =(DisplayName = "Overlay Effect"))
class SF_API USFAnimNotifyState_OverlayEffect : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	USFAnimNotifyState_OverlayEffect(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
	void ApplyEquipmentMeshComponent(FSFOverlayEffectProgressInfo& ProgressInfo, USkeletalMeshComponent* MeshComponent);
	void ApplyAllEquipmentMeshComponents(FSFOverlayEffectProgressInfo& ProgressInfo, USkeletalMeshComponent* MeshComponent);
	void ApplyCharacterMeshComponents(FSFOverlayEffectProgressInfo& ProgressInfo, USkeletalMeshComponent* MeshComponent);
	
protected:
	UPROPERTY(EditAnywhere)
	ESFOverlayTargetType OverlayTargetType = ESFOverlayTargetType::None;

	UPROPERTY(EditAnywhere)
	FGameplayTag EquipmentSlotTag;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UCurveLinearColor> LinearColorCurve;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UMaterialInterface> OverlayMaterial;

	UPROPERTY(EditAnywhere)
	FName ParameterName = "Color";

	UPROPERTY(EditAnywhere)
	FName ParameterAlpha = "FadeAlpha";

	UPROPERTY(EditAnywhere)
	bool bApplyRateScaleToProgress = true;
	
protected:
	UPROPERTY()
	TMap<TWeakObjectPtr<UMeshComponent>, FSFOverlayEffectProgressInfo> ProgressInfoMap;
};
