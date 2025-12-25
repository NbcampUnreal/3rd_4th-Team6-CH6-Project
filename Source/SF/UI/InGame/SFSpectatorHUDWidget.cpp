#include "SFSpectatorHUDWidget.h"

#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"
#include "Player/Components/SFSpectatorComponent.h"

void USFSpectatorHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (USFSpectatorComponent* SpectatorComp = GetSpectatorComponent())
	{
		SpectatorComp->OnSpectatorTargetChanged.AddDynamic(this, &ThisClass::OnSpectatorTargetChanged);

		// 이미 관전 중이면 초기값 표시
		if (APawn* CurrentTarget = SpectatorComp->GetCurrentSpectatorTarget())
		{
			OnSpectatorTargetChanged(CurrentTarget);
		}
	}
}

void USFSpectatorHUDWidget::NativeDestruct()
{
	if (USFSpectatorComponent* SpectatorComp = CachedSpectatorComponent.Get())
	{
		SpectatorComp->OnSpectatorTargetChanged.RemoveDynamic(this, &ThisClass::OnSpectatorTargetChanged);
	}
	
	Super::NativeDestruct();
}

void USFSpectatorHUDWidget::OnSpectatorTargetChanged(APawn* NewTarget)
{
	if (!Text_SpectatingPlayerName)
	{
		return;
	}

	if (NewTarget)
	{
		if (APlayerState* PS = NewTarget->GetPlayerState())
		{
			Text_SpectatingPlayerName->SetText(FText::FromString(PS->GetPlayerName()));
		}
	}
	else
	{
		Text_SpectatingPlayerName->SetText(FText::FromString(TEXT("No players alive")));
	}
}

USFSpectatorComponent* USFSpectatorHUDWidget::GetSpectatorComponent() const
{
	if (CachedSpectatorComponent.IsValid())
	{
		return CachedSpectatorComponent.Get();
	}

	if (APlayerController* PC = GetOwningPlayer())
	{
		USFSpectatorComponent* SpectatorComp = PC->FindComponentByClass<USFSpectatorComponent>();
		CachedSpectatorComponent = SpectatorComp;
		return SpectatorComp;
	}

	return nullptr;
}

