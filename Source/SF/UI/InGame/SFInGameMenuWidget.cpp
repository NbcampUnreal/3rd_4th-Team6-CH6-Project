#include "UI/InGame/SFInGameMenuWidget.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

#include "UI/Common/CommonButtonBase.h"

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
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
	}
}

void USFInGameMenuWidget::OnOptionsClicked()
{
	// TODO : Options 메뉴 구현시 연결 예정
	return;
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
