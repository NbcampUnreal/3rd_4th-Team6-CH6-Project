#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SFStageAnnouncementWidget.generated.h"

class UTextBlock;
class UWidgetAnimation;

UCLASS()
class SF_API USFStageAnnouncementWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	
	// 외부에서 이 함수를 호출해 연출을 시작
	UFUNCTION(BlueprintCallable, Category = "SF|UI")
	void ShowStageAnnouncement(const FText& StageName);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_StageName;
	
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> Anim_FadeInOut;

	UFUNCTION()
	void OnAnnounceFinished();
};
