#pragma once

#include "CoreMinimal.h"
#include "UI/SFUserWidget.h"
#include "SFDamageScreenWidget.generated.h"

class UImage;
class UWidgetAnimation;
class UMaterialInterface;

UCLASS()
class SF_API USFDamageScreenWidget : public USFUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnWidgetControllerSet() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void OnDamageReceived(float NewValue);

	void PlayDamageEffect();
	void PlayFadeOut();
	void ScheduleFadeOut();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_Blood;

	// FadeIn 애니메이션
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> Anim_FadeIn;

	UPROPERTY(EditAnywhere, Category = "SF|DamageScreen")
	TArray<TObjectPtr<UMaterialInterface>> BloodMaterials;

	// FadeIn 후 FadeOut 시작까지 대기 시간
	UPROPERTY(EditDefaultsOnly, Category = "SF|DamageScreen")
	float FadeOutDelay = 2.0f;

private:
	// 현재 재생 중인 시퀀스
	UPROPERTY()
	TObjectPtr<UUMGSequencePlayer> ActiveSequencePlayer;

	// FadeOut 타이머
	FTimerHandle FadeOutTimerHandle;
};
