#include "UI/InGame/SFInGameMenuWidget.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

#include "UI/Common/CommonButtonBase.h"
#include "Player/Lobby/SFLobbyPlayerController.h"

void USFInGameMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 버튼 클릭시 함수 연결
	if (ResumeButton) ResumeButton->OnButtonClickedDelegate.AddDynamic(this, &USFInGameMenuWidget::OnResumeClicked);
	if (OptionsButton) OptionsButton->OnButtonClickedDelegate.AddDynamic(this, &USFInGameMenuWidget::OnOptionsClicked);
	if (ReturnToTitleButton) ReturnToTitleButton->OnButtonClickedDelegate.AddDynamic(this, &USFInGameMenuWidget::OnReturnToTitleClicked);
	if (QuitGameButton) QuitGameButton->OnButtonClickedDelegate.AddDynamic(this, &USFInGameMenuWidget::OnQuitGameClicked);
}

FReply USFInGameMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		// 닫기 로직 실행
		if (ResumeButton)
		{
			ResumeButton->OnButtonClicked();
		}
		// 처리 완료 했음을 리턴
		return FReply::Handled();
	}
	
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}


void USFInGameMenuWidget::OnResumeClicked()
{
	RemoveFromParent();

	if (APlayerController* PC = GetOwningPlayer())
	{
		// 1. 로비 컨트롤러 = 로비 레벨 -> (IsA를 사용해 타입 검사)
		if (PC->IsA(ASFLobbyPlayerController::StaticClass()))
		{
			FInputModeGameAndUI InputMode;
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			InputMode.SetHideCursorDuringCapture(false);
			
			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = true;
		}
		// 2. 그 외(인게임)
		else
		{
			PC->SetInputMode(FInputModeGameOnly());
			PC->bShowMouseCursor = false;
		}
	}
}

void USFInGameMenuWidget::OnOptionsClicked()
{
	if (!OptionsWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("OptionsWidgetClass 가 UInGameMenuWidget BP에서 설정되지 않았습니다."));
		return;
	}
	
	UUserWidget* OptionsWidget = CreateWidget<UUserWidget>(this, OptionsWidgetClass);

	if (OptionsWidget)
	{
		OptionsWidget->AddToViewport(100);

		OptionsWidget->SetKeyboardFocus();
	}
}

void USFInGameMenuWidget::OnReturnToTitleClicked()
{
	// 맵 설정되어 있는지 확인 코드
	if (MainMenuLevelAsset.IsNull())
	{
		UE_LOG(LogTemp, Error, TEXT("인게임 메뉴의 메인메뉴 레벨이 설정되지 않았습니다."));
		return;
	}

	FString MapName = MainMenuLevelAsset.GetLongPackageName();
	UGameplayStatics::OpenLevel(this, FName(*MapName));
}

void USFInGameMenuWidget::OnQuitGameClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, true);
}
