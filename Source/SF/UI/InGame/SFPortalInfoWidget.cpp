#include "SFPortalInfoWidget.h"

#include "SFLogChannels.h"
#include "SFPortalInfoEntryWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "GameModes/SFGameState.h"
#include "Messages/SFMessageGameplayTags.h"
#include "Messages/SFPortalInfoMessages.h"
#include "Player/SFPlayerState.h"

void USFPortalInfoWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 초기 상태는 숨김
    SetVisibility(ESlateVisibility::Collapsed);
    Text_Countdown->SetVisibility(ESlateVisibility::Collapsed);

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // GameState 델리게이트 바인딩
    ASFGameState* SFGameState = World->GetGameState<ASFGameState>();
    if (SFGameState)
    {
        // 델리게이트에 함수 바인딩
        SFGameState->OnPlayerAdded.AddDynamic(this, &USFPortalInfoWidget::HandlePlayerAdded);
        SFGameState->OnPlayerRemoved.AddDynamic(this, &USFPortalInfoWidget::HandlePlayerRemoved);

        // 이미 접속해 있는 플레이어 Entry 등록
        for (APlayerState* PS : SFGameState->PlayerArray)
        {
            CreateEntryForPlayer(PS);
        }

        RefreshEntryOrder();
    }

    // GMS 리스너 등록
    UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(World);
    
    // Portal Manager의 글로벌 PortalState 변경 메시지 리스너로 등록
    PortalInfoListenerHandle = MessageSubsystem.RegisterListener(
        SFGameplayTags::Message_Portal_StateChanged, 
        this, 
        &USFPortalInfoWidget::HandlePortalInfoChanged);

    // 플레이어별 준비 상태 변경에 대한 리스너로 등록
    PlayerReadyListenerHandle = MessageSubsystem.RegisterListener(
        SFGameplayTags::Message_Player_TravelReadyChanged, 
        this, 
        &USFPortalInfoWidget::HandlePlayerReadyChanged);

    // Dead 상태 변경 리스너 등록
    DeadStateListenerHandle = MessageSubsystem.RegisterListener(
        SFGameplayTags::Message_Player_DeadStateChangedUI,
        this,
        &USFPortalInfoWidget::HandlePlayerDeadStateChanged);
}

void USFPortalInfoWidget::NativeDestruct()
{
    // 리스너 및 델리게이트 정리
    if (UWorld* World = GetWorld())
    {
        if (ASFGameState* SFGameState = World->GetGameState<ASFGameState>())
        {
            SFGameState->OnPlayerAdded.RemoveAll(this);
            SFGameState->OnPlayerRemoved.RemoveAll(this);
        }
        if (UGameplayMessageSubsystem::HasInstance(this))
        {
            UGameplayMessageSubsystem& GMS = UGameplayMessageSubsystem::Get(World);
            GMS.UnregisterListener(PortalInfoListenerHandle);
            GMS.UnregisterListener(PlayerReadyListenerHandle);
            GMS.UnregisterListener(DeadStateListenerHandle);
        }
    }
    
    Super::NativeDestruct();
}

void USFPortalInfoWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (bIsCountingDown)
    {
        CurrentCountdownTime -= InDeltaTime;
        if (CurrentCountdownTime <= 0.0f)
        {
            CurrentCountdownTime = 0.0f;
        }

        // 텍스트 갱신
        UpdateCountdownText();
    }
}

void USFPortalInfoWidget::HandlePlayerAdded(APlayerState* PlayerState)
{
    if (CreateEntryForPlayer(PlayerState))
    {
        RefreshEntryOrder();
    }
}

void USFPortalInfoWidget::HandlePlayerRemoved(APlayerState* PlayerState)
{
    if (!PlayerState || !PortalEntryMap.Contains(PlayerState))
    {
        return;
    }

    USFPortalInfoEntryWidget* EntryToRemove = PortalEntryMap.FindRef(PlayerState);
    
    // PortalEntryBox에서 Entry 제거
    if (EntryToRemove)
    {
        EntryToRemove->RemoveFromParent();
        PortalEntryMap.Remove(PlayerState);
    }
}

bool USFPortalInfoWidget::CreateEntryForPlayer(APlayerState* PlayerState)
{
    if (!PlayerState || PortalEntryMap.Contains(PlayerState))
    {
        return false;
    }

    if (!PortalEntryClass)
    {
        UE_LOG(LogSF, Warning, TEXT("USFPortalInfoWidget: PortalEntryClass is not set."));
        return false;
    }

    ASFPlayerState* SFPlayerState = Cast<ASFPlayerState>(PlayerState);
    if (!SFPlayerState)
    {
        return false;
    }

    USFPortalInfoEntryWidget* NewPortalEntry = CreateWidget<USFPortalInfoEntryWidget>(this, PortalEntryClass);
    if (NewPortalEntry)
    {
        NewPortalEntry->InitializeRow(SFPlayerState); 
        PortalEntryMap.Add(PlayerState, NewPortalEntry);
        return true;
    }
    
    return false;
}


void USFPortalInfoWidget::RefreshEntryOrder()
{
    if (!PortalEntryBox)
    {
        return;
    }

    TArray<TPair<APlayerState*, USFPortalInfoEntryWidget*>> SortedEntries;
    for (auto& Elem : PortalEntryMap)
    {
        APlayerState* PS = Elem.Key.Get();    // TObjectPtr → raw pointer 자동 변환
        USFPortalInfoEntryWidget* Widget = Elem.Value;
        
        if (PS && Widget)
        {
            SortedEntries.Emplace(PS, Widget);
        }
    }

    // PlayerId 기준 정렬
    SortedEntries.Sort([](const TPair<APlayerState*, USFPortalInfoEntryWidget*>& A, const TPair<APlayerState*, USFPortalInfoEntryWidget*>& B)
    {
        return A.Key->GetPlayerId() < B.Key->GetPlayerId();
    });

    PortalEntryBox->ClearChildren();

    // 정렬된 순서로 다시 추가
    for (const auto& Entry : SortedEntries)
    {
        PortalEntryBox->AddChild(Entry.Value);
    }
}

void USFPortalInfoWidget::HandlePortalInfoChanged(FGameplayTag Channel, const FSFPortalStateMessage& Message)
{
    // Message.bIsActive는 최초로 포탈로 이동한 플레이어가 존재하여 UI를 표시해야 함을 의미함. 
    if (Message.bIsActive)
    {
        SetVisibility(ESlateVisibility::Visible);
    }
    else
    {
        SetVisibility(ESlateVisibility::Collapsed);
    }

    // 맵 이동 카운트 다운 텍스트 표시
    if (Message.TravelCountdown > 0.0f)
    {
        bIsCountingDown = true;

        // TODO : 서버에서 클라로 메시지 도착 시간에 따른 카운트다운 시간 조정 필요
        CurrentCountdownTime = Message.TravelCountdown;
        Text_Countdown->SetVisibility(ESlateVisibility::Visible);
        UpdateCountdownText();
    }
    else
    {
        bIsCountingDown = false;
        Text_Countdown->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void USFPortalInfoWidget::HandlePlayerReadyChanged(FGameplayTag Channel, const FSFPlayerTravelReadyMessage& Message)
{
    if (!Message.PlayerState)
    {
        return;
    }

    // TMap에서 해당 플레이어의 Entry 위젯 찾기
    if (USFPortalInfoEntryWidget* EntryToUpdate = PortalEntryMap.FindRef(Message.PlayerState))
    {
        // Entry 위젯 내부의 함수를 호출하여 '준비' UI만 업데이트
        EntryToUpdate->SetReadyStatus(Message.bIsReadyToTravel);
    }
}

void USFPortalInfoWidget::HandlePlayerDeadStateChanged(FGameplayTag Channel, const FSFPlayerDeadStateMessage& Message)
{
    if (USFPortalInfoEntryWidget* Entry = PortalEntryMap.FindRef(Message.PlayerState))
    {
        Entry->SetDeadStatus(Message.bIsDead);
    }
}

void USFPortalInfoWidget::UpdateCountdownText()
{
    if (Text_Countdown)
    {
        int32 DisplayTime = FMath::CeilToInt(CurrentCountdownTime);
        Text_Countdown->SetText(FText::AsNumber(DisplayTime));
    }
}
