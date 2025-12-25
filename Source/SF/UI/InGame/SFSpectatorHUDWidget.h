#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SFSpectatorHUDWidget.generated.h"

class USFSpectatorComponent;
class UTextBlock;

UCLASS()
class SF_API USFSpectatorHUDWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void OnSpectatorTargetChanged(APawn* NewTarget);

	UFUNCTION(BlueprintPure, Category = "UI|Spectator")
	USFSpectatorComponent* GetSpectatorComponent() const;

protected:
	// 관전 대상 이름
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_SpectatingPlayerName;

private:
	UPROPERTY()
	mutable TWeakObjectPtr<USFSpectatorComponent> CachedSpectatorComponent;
};
