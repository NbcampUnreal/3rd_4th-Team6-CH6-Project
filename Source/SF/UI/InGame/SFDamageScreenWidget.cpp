#include "SFDamageScreenWidget.h"

#include "SFLogChannels.h"
#include "UI/Controller/SFOverlayWidgetController.h"
#include "Components/Image.h"
#include "Animation/UMGSequencePlayer.h"
#include "Materials/MaterialInterface.h"

void USFDamageScreenWidget::NativeOnWidgetControllerSet()
{
    Super::NativeOnWidgetControllerSet();

    UE_LOG(LogSF, Warning, TEXT("BloodMaterials.Num() = %d"), BloodMaterials.Num());
    UE_LOG(LogSF, Warning, TEXT("Widget Class: %s"), *GetClass()->GetName());

    if (Img_Blood)
    {
        Img_Blood->SetRenderOpacity(0.f);
    }

    if (USFOverlayWidgetController* OverlayController = GetWidgetControllerTyped<USFOverlayWidgetController>())
    {
        OverlayController->OnDamageReceived.AddDynamic(this, &ThisClass::OnDamageReceived);
    }
}

void USFDamageScreenWidget::NativeDestruct()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(FadeOutTimerHandle);
    }

    if (USFOverlayWidgetController* OverlayController = GetWidgetControllerTyped<USFOverlayWidgetController>())
    {
        OverlayController->OnDamageReceived.RemoveDynamic(this, &ThisClass::OnDamageReceived);
    }

    Super::NativeDestruct();
}

void USFDamageScreenWidget::OnDamageReceived(float DamageAmount)
{
    // FadeOut 재생 중이면 Reverse하여 다시 FadeIn
    if (ActiveSequencePlayer && ActiveSequencePlayer->GetPlaybackStatus() == EMovieScenePlayerStatus::Playing)
    {
        if (!ActiveSequencePlayer->IsPlayingForward())
        {
            ActiveSequencePlayer->Reverse();
        }
        ScheduleFadeOut();
        return;
    }

    // FadeOut 대기 중이면 타이머만 리셋
    if (GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(FadeOutTimerHandle))
    {
        ScheduleFadeOut();
        return;
    }

    PlayDamageEffect();
}

void USFDamageScreenWidget::PlayDamageEffect()
{
    // 랜덤 머티리얼 선택
    if (BloodMaterials.Num() > 0 && Img_Blood)
    {
        int32 RandomIndex = FMath::RandRange(0, BloodMaterials.Num() - 1);
        if (UMaterialInterface* SelectedMaterial = BloodMaterials[RandomIndex])
        {
            Img_Blood->SetBrushFromMaterial(SelectedMaterial);
        }
    }

    // FadeIn 애니메이션 재생
    if (Anim_FadeIn)
    {
        ActiveSequencePlayer = PlayAnimationForward(Anim_FadeIn);
    }

    ScheduleFadeOut();
}

void USFDamageScreenWidget::PlayFadeOut()
{
    if (Anim_FadeIn)
    {
        ActiveSequencePlayer = PlayAnimationReverse(Anim_FadeIn);
    }
}

void USFDamageScreenWidget::ScheduleFadeOut()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            FadeOutTimerHandle,
            this,
            &ThisClass::PlayFadeOut,
            FadeOutDelay,
            false
        );
    }
}