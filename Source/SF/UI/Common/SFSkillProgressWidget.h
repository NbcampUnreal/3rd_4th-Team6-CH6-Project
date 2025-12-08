#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "SFSkillProgressWidget.generated.h"

class UProgressBar;
class UTextBlock;
struct FSFSkillProgressRefreshMessage;
struct FSFSkillProgressInfoMessage;

UCLASS()
class SF_API USFSkillProgressWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	USFSkillProgressWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	void ConstructUI(FGameplayTag Channel, const FSFSkillProgressInfoMessage& Message);
	void RefreshUI(FGameplayTag Channel, const FSFSkillProgressRefreshMessage& Message);

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_SkillName;
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UProgressBar> ProgressBar_SkillProgress;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_RemainTime;
	
protected:
	UPROPERTY(EditAnywhere, meta=(Categories="SF|Message"))
	FGameplayTag ConstructMessageChannelTag;

	UPROPERTY(EditAnywhere, meta=(Categories="SF|Message"))
	FGameplayTag RefreshMessageChannelTag;

private:
	float PassedSkillTime = 0.f;
	float TargetSkillTime = 0.f;
	
	FGameplayMessageListenerHandle ConstructListenerHandle;
	FGameplayMessageListenerHandle RefreshListenerHandle;
};
