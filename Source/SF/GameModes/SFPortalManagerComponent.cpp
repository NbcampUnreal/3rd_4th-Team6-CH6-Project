#include "SFPortalManagerComponent.h"
#include "Actors/SFPortal.h"
#include "GameModes/SFGameMode.h"
#include "Player/SFPlayerState.h"
#include "SFLogChannels.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/GameStateBase.h"
#include "Messages/SFMessageGameplayTags.h"

USFPortalManagerComponent::USFPortalManagerComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	bPortalActive = false;
	bIsTraveling = false;
    TravelCountdownRemaining = -1.0f;
}

void USFPortalManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USFPortalManagerComponent, bPortalActive);
}

void USFPortalManagerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void USFPortalManagerComponent::ActivatePortal()
{
    if (!GetOwner()->HasAuthority() || bPortalActive)
    {
        return;
    }

    bPortalActive = true;

    if (ManagedPortal)
    {
        ManagedPortal->SetPortalEnabled(true);
    }
    
    // 활성화 메시지 브로드캐스트
    BroadcastPortalState();
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

    const FUniqueNetIdRepl& UniqueId = PlayerState->GetUniqueId();
    if (!UniqueId.IsValid())
    {
        return;
    }

    // 이미 포탈에 있으면 무시
    bool bAlreadyAdded;
    PlayersInsidePortal.Add(UniqueId, &bAlreadyAdded);

    if (bAlreadyAdded)
    {
        return;
    }
    
    // 상태 브로드캐스트
    BroadcastPortalState();
    
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

    const FUniqueNetIdRepl& UniqueId = PlayerState->GetUniqueId();
    if (!UniqueId.IsValid())
    {
        return;
    }

    if (PlayersInsidePortal.Remove(UniqueId) > 0)
    {
        // 타이머 취소
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(TravelTimerHandle);
        }
        TravelCountdownRemaining = -1.0f;
        
        // 상태 브로드캐스트
        BroadcastPortalState();
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

APlayerState* USFPortalManagerComponent::FindPlayerStateByUniqueId(const FUniqueNetIdRepl& UniqueId) const
{
    if (!UniqueId.IsValid())
    {
        return nullptr;
    }

    const AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>();
    for (APlayerState* PS : GameState->PlayerArray)
    {
        if (PS && PS->GetUniqueId() == UniqueId)
        {
            return PS;
        }
    }
    
    return nullptr;
}

void USFPortalManagerComponent::RegisterPortal(ASFPortal* Portal)
{
    if (!Portal)
    {
        return;
    }

    ManagedPortal = Portal;
    
    // 현재 활성화 상태에 맞춰 Portal 업데이트
    ManagedPortal->SetPortalEnabled(bPortalActive);
}

void USFPortalManagerComponent::UnregisterPortal(ASFPortal* Portal)
{
    if (Portal && ManagedPortal == Portal)
    {
        ManagedPortal = nullptr;
    }
}

void USFPortalManagerComponent::CheckPortalReadyAndTravel()
{
    const int32 RequiredCount = GetRequiredPlayerCount();
    
    if (!bPortalActive || PlayersInsidePortal.Num() < RequiredCount || RequiredCount <= 0)
    {
        return;
    }

    // 카운트다운 시작
    TravelCountdownRemaining = TravelDelayTime;
    
    // 상태 브로드캐스트 (카운트다운 UI 표시)
    BroadcastPortalState();

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
    const int32 RequiredCount = GetRequiredPlayerCount();
    
    if (PlayersInsidePortal.Num() < RequiredCount)
    {
        return;
    }

    bIsTraveling = true;

    if (ASFGameMode* SFGameMode = GetGameMode<ASFGameMode>())
    {
        if (ManagedPortal && !ManagedPortal->GetNextStageLevel().IsNull())
        {
            SFGameMode->RequestTravelToNextStage(ManagedPortal->GetNextStageLevel());
        }
    }
}

void USFPortalManagerComponent::BroadcastPortalState()
{
    UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
    
    FSFPortalStateMessage Message;
    Message.bIsActive = bPortalActive;
    Message.TotalPlayerCount = GetRequiredPlayerCount();
    Message.TravelCountdown = TravelCountdownRemaining;
    
    Message.PlayersInPortal.Reserve(PlayersInsidePortal.Num());
    for (const FUniqueNetIdRepl& UniqueId : PlayersInsidePortal)
    {
        if (APlayerState* PS = FindPlayerStateByUniqueId(UniqueId))
        {
            Message.PlayersInPortal.Add(PS);
        }
    }
    
    Message.bReadyToTravel = (Message.PlayersInPortal.Num() >= Message.TotalPlayerCount) && 
                              Message.TotalPlayerCount > 0;
    
    MessageSubsystem.BroadcastMessage(SFGameplayTags::Message_Portal_InfoChanged, Message);
}

void USFPortalManagerComponent::OnRep_PortalActive()
{
    if (ManagedPortal)
    {
        ManagedPortal->SetPortalEnabled(bPortalActive);
    }
}
