#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "SFGameClearOverlayWidget.generated.h"

struct FSFGameClearMessage;
struct FSFGameOverResult;
class UTextBlock;

/**
 * 
 */
UCLASS()
class SF_API USFGameClearOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	// 게임 클리어 메시지 수신 → 오버레이 표시
	void OnGameClearReceived(FGameplayTag Channel, const FSFGameClearMessage& Message);

	// 통계 메시지 수신 → 오버레이 숨김 (통계창이 표시됨)
	void OnGameOverStatsReceived(FGameplayTag Channel, const FSFGameOverResult& Result);

	void UpdateCountdownDisplay();
	float GetRemainingStatsTime() const;

protected:

	// "통계 표시까지 XX초" 카운트다운
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Countdown;

	// 애니메이션 (선택)
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> Anim_GameClear;

private:
	FGameplayMessageListenerHandle GameClearListenerHandle;
	FGameplayMessageListenerHandle StatsListenerHandle;

	float TargetStatsTime = 0.f;
	bool bIsActive = false;
};
