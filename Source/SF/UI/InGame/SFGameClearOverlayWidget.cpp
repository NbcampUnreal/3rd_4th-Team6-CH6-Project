#include "SFGameClearOverlayWidget.h"

#include "Components/TextBlock.h"
#include "GameFramework/GameStateBase.h"
#include "Messages/SFMessageGameplayTags.h"
#include "Messages/SFPortalInfoMessages.h"
#include "Player/SFPlayerInfoTypes.h"

void USFGameClearOverlayWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 초기 숨김
    SetVisibility(ESlateVisibility::Collapsed);

    if (UGameplayMessageSubsystem::HasInstance(this))
    {
        UGameplayMessageSubsystem& GMS = UGameplayMessageSubsystem::Get(this);
        
        // 게임 클리어 시 오버레이 표시
        GameClearListenerHandle = GMS.RegisterListener(SFGameplayTags::Message_Game_GameClear,this, &ThisClass::OnGameClearReceived);

        // 통계 표시 시 오버레이 숨김
        StatsListenerHandle = GMS.RegisterListener(SFGameplayTags::Message_Game_GameOverStats,this,&ThisClass::OnGameOverStatsReceived);
    }
}

void USFGameClearOverlayWidget::NativeDestruct()
{
    if (UGameplayMessageSubsystem::HasInstance(this))
    {
        UGameplayMessageSubsystem& GMS = UGameplayMessageSubsystem::Get(this);
        GMS.UnregisterListener(GameClearListenerHandle);
        GMS.UnregisterListener(StatsListenerHandle);
    }

    Super::NativeDestruct();
}

void USFGameClearOverlayWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (bIsActive)
    {
        UpdateCountdownDisplay();
    }
}

void USFGameClearOverlayWidget::OnGameClearReceived(FGameplayTag Channel, const FSFGameClearMessage& Message)
{
    TargetStatsTime = Message.TargetStatsTime;
    bIsActive = true;

    SetVisibility(ESlateVisibility::SelfHitTestInvisible);

    // 애니메이션 재생
    if (Anim_GameClear)
    {
        PlayAnimation(Anim_GameClear);
    }

    UpdateCountdownDisplay();
}

void USFGameClearOverlayWidget::OnGameOverStatsReceived(FGameplayTag Channel, const FSFGameOverResult& Result)
{
    // 통계창이 표시되므로 오버레이 숨김
    bIsActive = false;
    SetVisibility(ESlateVisibility::Collapsed);
}

void USFGameClearOverlayWidget::UpdateCountdownDisplay()
{
    if (!Text_Countdown)
    {
        return;
    }

    float RemainingTime = GetRemainingStatsTime();
    int32 Seconds = FMath::CeilToInt(RemainingTime);

    if (Seconds > 0)
    {
        Text_Countdown->SetText(FText::Format(
            NSLOCTEXT("GameClear", "StatsCountdown", "{0}초 후 결과 표시"),
            FText::AsNumber(Seconds)));
    }
    else
    {
        Text_Countdown->SetText(NSLOCTEXT("GameClear", "Loading", "결과 로딩 중..."));
    }
}

float USFGameClearOverlayWidget::GetRemainingStatsTime() const
{
    if (const UWorld* World = GetWorld())
    {
        if (const AGameStateBase* GS = World->GetGameState<AGameStateBase>())
        {
            return FMath::Max(0.f, TargetStatsTime - GS->GetServerWorldTimeSeconds());
        }
    }
    return 0.f;
}