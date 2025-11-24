#include "UI/MainMenu/MainMenuWidget.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

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
	/*if (!LobbyMapAsset.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("LobbyMapAsset 이 UMainMenuWidget BP에서 설정되지 않았습니다."));
		return;
	}*/

	// 1. 맵 이름 가져오기
	FString MapName = LobbyMapAsset.GetLongPackageName();

	// 2. 멀티플레이 세션 생성을 위한 옵션 정의
	// -> 해당 맵을 listen 서버로 열기 위하여 ?listen 옵션 추가 -> 다른 클라이언트 접속을 위한 호스트 지정
	FString TravelURL = FString::Printf(TEXT("%s?listen"), *MapName);

	// 3. 서버 맵 이동 (월드 트레블)
	if (UWorld* World = GetWorld())
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0 );

		if (PC && TravelURL != "")
		{
			// PC 서버(월드) 이동 요청 
			PC->ClientTravel(TravelURL, TRAVEL_Absolute);

			UE_LOG(LogTemp , Warning, TEXT("서버 열기 요청 : %s"), *TravelURL);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("New Game 버튼 클릭: 로비 맵으로 이동 시도"));
}

void UMainMenuWidget::OnSearchMatchClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("SearchMatch 버튼 클릭: 세션 찾기 맵으로 이동 시도"));
}

void UMainMenuWidget::OnOptionsClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Options 버튼 클릭: 옵션 창으로 이동 시도"));
}

void UMainMenuWidget::OnCreditsClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Credits 버튼 클릭: 크레딧 창으로 이동 시도"));
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
