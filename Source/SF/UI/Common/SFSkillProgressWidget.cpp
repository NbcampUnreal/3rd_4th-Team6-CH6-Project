#include "SFSkillProgressWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetMathLibrary.h"
#include "Messages/SFSkillInfoMessages.h"

USFSkillProgressWidget::USFSkillProgressWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

void USFSkillProgressWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetVisibility(ESlateVisibility::Collapsed);
	
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	ConstructListenerHandle = MessageSubsystem.RegisterListener(ConstructMessageChannelTag, this, &ThisClass::ConstructUI);
	RefreshListenerHandle = MessageSubsystem.RegisterListener(RefreshMessageChannelTag, this, &ThisClass::RefreshUI);
}

void USFSkillProgressWidget::NativeDestruct()
{
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	MessageSubsystem.UnregisterListener(ConstructListenerHandle);
	MessageSubsystem.UnregisterListener(RefreshListenerHandle);
	
	Super::NativeDestruct();
}

void USFSkillProgressWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	if (GetVisibility() == ESlateVisibility::Visible)
	{
		PassedSkillTime = FMath::Min(PassedSkillTime + InDeltaTime, TargetSkillTime);
		ProgressBar_SkillProgress->SetPercent(UKismetMathLibrary::SafeDivide(PassedSkillTime, TargetSkillTime));

		FNumberFormattingOptions Options;
		Options.MinimumFractionalDigits = 1;
		Options.MaximumFractionalDigits = 1;
		Text_RemainTime->SetText(FText::AsNumber(FMath::Clamp(TargetSkillTime - PassedSkillTime, 0.f, TargetSkillTime), &Options));
	}
}

void USFSkillProgressWidget::ConstructUI(FGameplayTag Channel, const FSFSkillProgressInfoMessage& Message)
{
	if (Message.bShouldShow)
	{
		PassedSkillTime = 0.f;
		TargetSkillTime = Message.TotalSkillTime;

		Text_SkillName->SetText(Message.DisplayName);
		ProgressBar_SkillProgress->SetPercent(0.f);
		ProgressBar_SkillProgress->SetFillColorAndOpacity(Message.PhaseColor);
		SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}
}

void USFSkillProgressWidget::RefreshUI(FGameplayTag Channel, const FSFSkillProgressRefreshMessage& Message)
{
	if (GetVisibility() == ESlateVisibility::Visible)
	{
		ProgressBar_SkillProgress->SetFillColorAndOpacity(Message.PhaseColor);
	}
}
