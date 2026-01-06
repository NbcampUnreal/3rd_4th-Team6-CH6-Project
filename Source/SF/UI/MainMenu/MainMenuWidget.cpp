#include "UI/MainMenu/MainMenuWidget.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Blueprint/UserWidget.h"

#include "UI/SearchLobby/SFCreateRoomWidget.h"
#include "UI/SearchLobby/SFSearchLobbyWidget.h"
#include "UI/MainMenu/SFCreditWidget.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(Btn_NewGame))
	{
		Btn_NewGame->OnButtonClickedDelegate.AddDynamic(this, &UMainMenuWidget::OnNewGameClicked);
	}

	if (IsValid(Btn_SearchMatch))
	{
		Btn_SearchMatch->OnButtonClickedDelegate.AddDynamic(this, &UMainMenuWidget::OnSearchMatchClicked);
	}

	if (IsValid(Btn_Options))
	{
		Btn_Options->OnButtonClickedDelegate.AddDynamic(this, &UMainMenuWidget::OnOptionsClicked);
	}

	if (IsValid(Btn_Credits))
	{
		Btn_Credits->OnButtonClickedDelegate.AddDynamic(this, &UMainMenuWidget::OnCreditsClicked);
	}
	
	if (IsValid(Btn_Quit))
	{
		Btn_Quit->OnButtonClickedDelegate.AddDynamic(this, &UMainMenuWidget::OnQuitClicked);
	}
}

void UMainMenuWidget::OnNewGameClicked()
{
	if (!CreateGameWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateGameWidgetClass 가 UMainMenuWidget BP에서 설정되지 않았습니다."));
		return;
	}

	USFCreateRoomWidget* CreateRoomWidgetClass = CreateWidget<USFCreateRoomWidget>(this, CreateGameWidgetClass);

	if (CreateRoomWidgetClass)
	{
		CreateRoomWidgetClass->AddToViewport(100);

		UE_LOG(LogTemp,Warning, TEXT("CreateRoomWidgetClass 생성 완료."));
	}
}

void UMainMenuWidget::OnSearchMatchClicked()
{
	if (!SearchLobbyWidgetClass) 
	{
		UE_LOG(LogTemp, Error, TEXT("SearchLobbyWidgetClass가 UMainMenuWidget BP에서 설정되지 않았습니다."));
		return;
	}
	USFSearchLobbyWidget* LobbyWidget = CreateWidget<USFSearchLobbyWidget>(this, SearchLobbyWidgetClass);

	if (LobbyWidget)
	{
		LobbyWidget->AddToViewport(100);

		UE_LOG(LogTemp,Warning, TEXT("SearchLobbyWidget 생성 완료."));
	}
}

void UMainMenuWidget::OnOptionsClicked()
{
	if (!OptionsWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("OptionsWidgetClass 가 UMainMenuWidget BP에서 설정되지 않았습니다."));
		return;
	}
	
	UUserWidget* OptionsWidget = CreateWidget<UUserWidget>(this, OptionsWidgetClass);

	if (OptionsWidget)
	{
		OptionsWidget->AddToViewport(100);

		OptionsWidget->SetKeyboardFocus();
	}
}

void UMainMenuWidget::OnCreditsClicked()
{
	if (!CreditsWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("CreditsWidgetClass 가 UMainMenuWidget BP에서 설정되지 않았습니다."));
		return;
	}

	USFCreditWidget* CreditWidget = CreateWidget<USFCreditWidget>(this, CreditsWidgetClass);

	if (CreditWidget)
	{
		CreditWidget->AddToViewport(100);
	}
}

void UMainMenuWidget::OnQuitClicked()
{
	if (UWorld* World = GetWorld())
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);

		if (PC)
		{
			UKismetSystemLibrary::QuitGame(World, PC, EQuitPreference::Quit, false);
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Quit 버튼 클릭 : 게임 종료 시도"));
}
