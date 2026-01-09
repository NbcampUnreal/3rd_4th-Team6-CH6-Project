// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SFInGameMenuWidget.generated.h"

class UCommonButtonBase;

UCLASS()
class SF_API USFInGameMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
public:
	// 메인메뉴 복귀 버튼을 누를시 이동 될 레벨 저장 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Game flow")
	TSoftObjectPtr<UWorld> MainMenuLevelAsset;

protected:
	// UMG의 버튼 연결
	UPROPERTY(meta = (BindWidget))
	UCommonButtonBase* ResumeButton;

	UPROPERTY(meta = (BindWidget))
	UCommonButtonBase* OptionsButton;
	
	UPROPERTY(meta = (BindWidget))
	UCommonButtonBase* ReturnToTitleButton;

	UPROPERTY(meta = (BindWidget))
	UCommonButtonBase* QuitGameButton;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI|GameFlow")
	TSubclassOf<UUserWidget> OptionsWidgetClass;

	UFUNCTION()
	void OnResumeClicked();

	UFUNCTION()
	void OnOptionsClicked();

	UFUNCTION()
	void OnReturnToTitleClicked();

	UFUNCTION()
	void OnQuitGameClicked();
};
