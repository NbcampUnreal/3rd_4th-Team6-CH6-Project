#include "SFPortalManagerComponent.h"
#include "Actors/SFPortal.h"
#include "GameModes/SFGameMode.h"
#include "Player/SFPlayerState.h"
#include "SFLogChannels.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/GameStateBase.h"

USFPortalManagerComponent::USFPortalManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	bPortalActive = false;
	bIsTraveling = false;
}

void USFPortalManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USFPortalManagerComponent, bPortalActive);
	DOREPLIFETIME(USFPortalManagerComponent, PlayersInPortal);
}

void USFPortalManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// 초기 상태 브로드캐스트
	BroadcastPlayerCountChanged();
}

void USFPortalManagerComponent::ActivatePortal()
{
	if (!GetOwner()->HasAuthority() || bPortalActive)
	{
		return;
	}

	bPortalActive = true;

	// 등록된 Portal들 비주얼 업데이트
	UpdatePortalVisuals(true);

	// Listen Server에서도 델리게이트 호출
	OnPortalActivated.Broadcast();
	BroadcastPlayerCountChanged();
}

void USFPortalManagerComponent::DeactivatePortal()
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	bPortalActive = false;
	PlayersInPortal.Empty();
	bIsTraveling = false;

	// 타이머 취소
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TravelTimerHandle);
	}

	// Portal 비주얼 업데이트
	UpdatePortalVisuals(false);

	BroadcastPlayerCountChanged();
}

void USFPortalManagerComponent::NotifyPlayerEnteredPortal(APlayerState* PlayerState)
{
	if (!GetOwner()->HasAuthority() || !bPortalActive || bIsTraveling)
	{
		return;
	}

	if (!PlayerState || PlayerState->IsInactive())
	{
		return;
	}

	// 이미 목록에 있는지 체크
	FSFPlayerSelectionInfo NewPlayerInfo = GetPlayerSelectionInfo(PlayerState);
	if (PlayersInPortal.Contains(NewPlayerInfo))
	{
		return;
	}

	// 플레이어 추가
	PlayersInPortal.Add(NewPlayerInfo);

	// 델리게이트 브로드캐스트
	OnPortalPlayerEntered.Broadcast(NewPlayerInfo);
	BroadcastPlayerCountChanged();
	
	// 모든 플레이어 체크
	CheckPortalReadyAndTravel();
}

void USFPortalManagerComponent::NotifyPlayerLeftPortal(APlayerState* PlayerState)
{
	if (!GetOwner()->HasAuthority() || bIsTraveling)
	{
		return;
	}

	if (!PlayerState)
	{
		return;
	}

	// 플레이어 찾기
	int32 FoundIndex = PlayersInPortal.IndexOfByPredicate([PlayerState](const FSFPlayerSelectionInfo& Info)
	{
		return Info.IsForPlayer(PlayerState);
	});

	if (FoundIndex != INDEX_NONE)
	{
		FSFPlayerSelectionInfo RemovedPlayer = PlayersInPortal[FoundIndex];
		PlayersInPortal.RemoveAt(FoundIndex);

		// 델리게이트 브로드캐스트
		OnPortalPlayerLeft.Broadcast(RemovedPlayer);
		BroadcastPlayerCountChanged();

		// 타이머 취소
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(TravelTimerHandle);
		}
	}
}

int32 USFPortalManagerComponent::GetRequiredPlayerCount() const
{
	const AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>();
	
	// 유효한 PlayerState만 카운트
	int32 ValidPlayerCount = 0;
	for (APlayerState* PS : GameState->PlayerArray)
	{
		if (PS && !PS->IsInactive())
		{
			ValidPlayerCount++;
		}
	}
	return ValidPlayerCount;
}

bool USFPortalManagerComponent::AreAllPlayersInPortal() const
{
	return bPortalActive && 
	       PlayersInPortal.Num() >= GetRequiredPlayerCount() && 
	       !bIsTraveling && 
	       GetRequiredPlayerCount() > 0;
}

void USFPortalManagerComponent::RegisterPortal(ASFPortal* Portal)
{
	if (!Portal)
	{
		return;
	}

	RegisteredPortals.AddUnique(Portal);
	
	// 현재 활성화 상태에 맞춰 Portal 업데이트
	Portal->SetPortalEnabled(bPortalActive);
}

void USFPortalManagerComponent::UnregisterPortal(ASFPortal* Portal)
{
	if (!Portal)
	{
		return;
	}

	RegisteredPortals.Remove(Portal);
}

void USFPortalManagerComponent::CheckPortalReadyAndTravel()
{
	if (!AreAllPlayersInPortal())
	{
		return;
	}

	UE_LOG(LogSF, Warning, TEXT("[PortalManager] All players ready! Starting travel in %.1f seconds"), 
		TravelDelayTime);

	// Travel 타이머 시작
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			TravelTimerHandle,
			this,
			&USFPortalManagerComponent::ExecuteTravel,
			TravelDelayTime,
			false
		);
	}
}

void USFPortalManagerComponent::ExecuteTravel()
{
	if (!AreAllPlayersInPortal())
	{
		return;
	}

	bIsTraveling = true;
	
	// GameMode에 Travel 요청
	if (UWorld* World = GetWorld())
	{
		if (ASFGameMode* GameMode = World->GetAuthGameMode<ASFGameMode>())
		{
			GameMode->RequestTravelToNextStage();
		}
	}
}

FSFPlayerSelectionInfo USFPortalManagerComponent::GetPlayerSelectionInfo(const APlayerState* PlayerState) const
{
	if (const ASFPlayerState* SFPS = Cast<ASFPlayerState>(PlayerState))
	{
		return SFPS->GetPlayerSelection();
	}

	FSFPlayerSelectionInfo DefaultInfo;
	if (PlayerState)
	{
		DefaultInfo = FSFPlayerSelectionInfo(0, PlayerState);
	}
	return DefaultInfo;
}

void USFPortalManagerComponent::UpdatePortalVisuals(bool bEnabled)
{
	for (ASFPortal* Portal : RegisteredPortals)
	{
		if (Portal)
		{
			Portal->SetPortalEnabled(bEnabled);
		}
	}
}

void USFPortalManagerComponent::BroadcastPlayerCountChanged()
{
	const int32 CurrentCount = PlayersInPortal.Num();
	const int32 RequiredCount = GetRequiredPlayerCount();
	
	OnPortalPlayerCountChanged.Broadcast(CurrentCount, RequiredCount);
}

void USFPortalManagerComponent::OnRep_PortalActive()
{
	if (bPortalActive)
	{
		OnPortalActivated.Broadcast();
	}
	
	// Portal 비주얼 업데이트 (클라이언트)
	UpdatePortalVisuals(bPortalActive);
	
	BroadcastPlayerCountChanged();
}

void USFPortalManagerComponent::OnRep_PlayersInPortal()
{
	BroadcastPlayerCountChanged();
}

