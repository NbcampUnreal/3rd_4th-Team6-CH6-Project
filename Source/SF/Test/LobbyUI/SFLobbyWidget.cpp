#include "SFLobbyWidget.h"

#include "SFHeroEntryWidget.h"
#include "Actors/SFHeroDisplay.h"
#include "Character/Hero/SFHeroDefinition.h"
#include "Components/Button.h"
#include "Components/TileView.h"
#include "Components/WidgetSwitcher.h"
#include "Engine/StreamableManager.h"
#include "GameFramework/PlayerStart.h"
#include "GameModes/Lobby/SFLobbyGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Player/Lobby/SFLobbyPlayerController.h"
#include "Player/Lobby/SFLobbyPlayerState.h"
#include "System/SFAssetManager.h"

void USFLobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();
	ConfigureGameState();
	SFLobbyPlayerController = GetOwningPlayer<ASFLobbyPlayerController>();
	if (SFLobbyPlayerController)
	{
		SFLobbyPlayerController->OnSwitchToHeroSelection.BindUObject(this, &USFLobbyWidget::SwitchToHeroSelection);
	}
	StartMatchButton->SetIsEnabled(false);
	StartMatchButton->OnClicked.AddDynamic(this, &ThisClass::StartMatchButtonClicked);
	
	USFAssetManager::Get().LoadHeroDefinitions(FStreamableDelegate::CreateUObject(this, &ThisClass::HeroDefinitionLoaded));

	if (HeroSelectionTileView)
	{
		HeroSelectionTileView->OnItemSelectionChanged().AddUObject(this, &USFLobbyWidget::HeroSelected);
	}

}

void USFLobbyWidget::ConfigureGameState()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	SFGameState = World->GetGameState<ASFLobbyGameState>();
	if (!SFGameState)
	{
		World->GetTimerManager().SetTimer(ConfigureGameStateTimerHandle, this, &ThisClass::ConfigureGameState, 1.f);
	}
	else
	{
		SFGameState->OnPlayerSelectionUpdated.AddUObject(this, &ThisClass::UpdatePlayerSelectionDisplay);
		UpdatePlayerSelectionDisplay(SFGameState->GetPlayerSelection());
	}
}

void USFLobbyWidget::UpdatePlayerSelectionDisplay(const TArray<FSFPlayerSelectionInfo>& PlayerSelections)
{
	// Refresh Hero Selection
	for (UUserWidget* HeroEntryAsWidget :HeroSelectionTileView->GetDisplayedEntryWidgets())
	{
		USFHeroEntryWidget* HeroEntryWidget = Cast<USFHeroEntryWidget>(HeroEntryAsWidget);
		if (HeroEntryWidget)
		{
			HeroEntryWidget->SetSelected(false);
		}
	}

	// Update Team Selection Slots
	for (const FSFPlayerSelectionInfo& PlayerSelection : PlayerSelections)
	{
		if (!PlayerSelection.IsValid())
		{
			continue;
		}

		// HeroDefintion과 연관된 HeroEntryWidget을 찾아서 선택된 상태로 업데이트
		USFHeroEntryWidget* SelectedEntry = HeroSelectionTileView->GetEntryWidgetFromItem<USFHeroEntryWidget>(PlayerSelection.GetHeroDefinition());
		if (SelectedEntry)
		{
			SelectedEntry->SetSelected(true);
		}

		// TODO : 핵심으로써 다른 플레이어의 캐릭터도 보여주도록 수정 해야함
		// if (PlayerSelection.IsForPlayer(GetOwningPlayerState()))
		// {
		// 	UpdateHeroDisplay(PlayerSelection);
		// }
	}

	if (SFGameState)
	{
		// 게임 시작 버튼
		StartMatchButton->SetIsEnabled(SFGameState->CanStartMatch());
	}
}

void USFLobbyWidget::SwitchToHeroSelection()
{
	MainSwitcher->SetActiveWidget(HeroSelectionRoot);
}

void USFLobbyWidget::HeroDefinitionLoaded()
{
	TArray<USFHeroDefinition*> LoadedCharacterDefinitions;
	if (USFAssetManager::Get().GetLoadedHeroDefinitions(LoadedCharacterDefinitions))
	{
		HeroSelectionTileView->SetListItems(LoadedCharacterDefinitions);
	}
}

void USFLobbyWidget::HeroSelected(UObject* SelectedUObject)
{
	if (!SFLobbyPlayerState)
	{
		SFLobbyPlayerState = GetOwningPlayerState<ASFLobbyPlayerState>();
	}

	if (!SFLobbyPlayerState)
	{
		return;
	}

	if (USFHeroDefinition* CharacterDefinition = Cast<USFHeroDefinition>(SelectedUObject))
	{
		SFLobbyPlayerState->Server_SetSelectedHeroDefinition(CharacterDefinition);
	}
}

void USFLobbyWidget::SpawnCharacterDisplay()
{
	if (HeroDisplay)
	{
		return;
	}
	if (!HeroDisplayClass)
	{
		return;
	}
	
	FTransform CharacterDisplayTransform = FTransform::Identity;

	// TODO : 접속한 플레이어별 PlayerStart 위치 지정 필요
	AActor* PlayerStart = UGameplayStatics::GetActorOfClass(GetWorld(), APlayerStart::StaticClass());
	if (PlayerStart)
	{
		CharacterDisplayTransform = PlayerStart->GetActorTransform();
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	HeroDisplay = GetWorld()->SpawnActor<ASFHeroDisplay>(HeroDisplayClass, CharacterDisplayTransform, SpawnParams);
	//GetOwningPlayer()->SetViewTarget(HeroDisplay);
}

void USFLobbyWidget::UpdateHeroDisplay(const FSFPlayerSelectionInfo& PlayerSelectionInfo)
{
	if (!PlayerSelectionInfo.GetHeroDefinition())
	{
		return;
	}

	//HeroDisplay->ConfigureWithHeroDefination(PlayerSelectionInfo.GetHeroDefinition());

	// TODO : CharacterInfo or PawnData 에서 캐릭터에 부여되는 Ability를 가져와서 AbilityListView에 적용되도록 구조 생각
	//AbilityListView->ConfigureAbilities();
}

void USFLobbyWidget::StartMatchButtonClicked()
{
	if (SFLobbyPlayerController)
	{
		SFLobbyPlayerController->Server_RequestStartMatch();
	}
}
