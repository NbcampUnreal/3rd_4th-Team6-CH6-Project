#include "Player/Components/SFSpectatorComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/Pawn.h"
#include "Player/SFPlayerState.h" 
#include "UI/InGame/SFSpectatorHUDWidget.h"
#include "UI/SearchLobby/SFCreateRoomWidget.h"

USFSpectatorComponent::USFSpectatorComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SetIsReplicatedByDefault(true);

}

void USFSpectatorComponent::BeginPlay()
{
    Super::BeginPlay();

    InitializeInputBindings();
}

void USFSpectatorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ToggleInputContext(false);
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(AutoSwitchTimerHandle);
        World->GetTimerManager().ClearTimer(SpectateRetryTimerHandle);
    }
    UnbindFromCurrentTarget();
    
    Super::EndPlay(EndPlayReason);
}

void USFSpectatorComponent::InitializeInputBindings()
{
    if (bInputBound)
    {
        return; 
    }
    
    APlayerController* PC = GetController<APlayerController>();
    if (!PC || !PC->IsLocalController())
    {
        return;
    }
    
    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PC->InputComponent))
    {
        if (SpectateNextAction)
        {
            EnhancedInput->BindAction(SpectateNextAction, ETriggerEvent::Started, this, &ThisClass::OnSpectateNextInput);
        }
        if (SpectatePreviousAction)
        {
            EnhancedInput->BindAction(SpectatePreviousAction, ETriggerEvent::Started, this, &ThisClass::OnSpectatePreviousInput);
        }
        
        bInputBound = true;
    }
}

void USFSpectatorComponent::ToggleInputContext(bool bEnable)
{
    APlayerController* PC = GetController<APlayerController>();
    if (!PC || !PC->IsLocalController() || !SpectatorMappingContext)
    {
        return;
    }
    
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
    {
        if (bEnable)
        {
            // Priority 1로 설정하여 기본 조작보다 우선순위를 높임
            Subsystem->AddMappingContext(SpectatorMappingContext, 1);
        }
        else
        {
            Subsystem->RemoveMappingContext(SpectatorMappingContext);
        }
    }
}

void USFSpectatorComponent::StartSpectating()
{
    if (bIsSpectating)
    {
        return;
    }
    
    bIsSpectating = true;

    ToggleInputContext(true);
    ShowSpectatorHUD();
    
    // 관전 시작 시엔 적절한 첫 번째 대상 관전
    TrySpectateNextPlayer();
}

void USFSpectatorComponent::TrySpectateNextPlayer()
{
    // 일단 다음 플레이어 찾기 시도
    SpectateNextPlayer();

    // 찾았는지 확인
    if (CurrentSpectatorTarget.IsValid())
    {
        // 성공! -> 재시도 타이머가 돌고 있었다면 끔
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().ClearTimer(SpectateRetryTimerHandle);
        }
    }
    else
    {
        // 실패 (아직 Pawn들이 스폰 안 됨) -> 타이머가 안 켜져 있다면 켬
        if (GetWorld() && !GetWorld()->GetTimerManager().IsTimerActive(SpectateRetryTimerHandle))
        {
            GetWorld()->GetTimerManager().SetTimer(
                SpectateRetryTimerHandle,
                this,
                &ThisClass::TrySpectateNextPlayer,
                0.5f,  // 0.5초마다 시도
                true   // 반복(Loop)
            );
        }
    }
}

void USFSpectatorComponent::StopSpectating()
{
    APlayerController* PC = GetController<APlayerController>();
    if (!PC || !bIsSpectating)
    {
        return;
    }
    ToggleInputContext(false);

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(AutoSwitchTimerHandle);
        World->GetTimerManager().ClearTimer(SpectateRetryTimerHandle);
    }

    UnbindFromCurrentTarget();
    HideSpectatorHUD();
    
    bIsSpectating = false;
    CurrentSpectatorTarget.Reset();

    // 내 원래 Pawn으로 복귀
    APawn* OwnedPawn = PC->GetPawn();
    
    // 서버 알림
    Server_SetViewTarget(OwnedPawn);

    // 로컬 복귀
    if (PC->IsLocalController() && OwnedPawn)
    {
        PC->SetViewTargetWithBlend(OwnedPawn, ViewTargetBlendTime, VTBlend_EaseInOut, 1.0f);
    }

    OnSpectatorTargetChanged.Broadcast(nullptr);
}

void USFSpectatorComponent::SpectateNextPlayer()
{
    if (!bIsSpectating)
    {
        return;
    }
    
    TArray<APawn*> Targets;
    CollectAliveTargets(Targets);

    if (Targets.Num() == 0)
    {
        return;
    }
    // 현재 타겟의 인덱스를 찾아서 다음 순번 결정
    int32 CurrentIndex = Targets.Find(CurrentSpectatorTarget.Get());
    
    // 못 찾았거나(처음 진입), 마지막이면 0번으로(앞으로 순환)
    int32 NewIndex = (CurrentIndex + 1) % Targets.Num();
    
    SetSpectatorTarget(Targets[NewIndex]);

    if (CurrentSpectatorTarget.IsValid() && GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(SpectateRetryTimerHandle);
    }
}

void USFSpectatorComponent::SpectatePreviousPlayer()
{
    if (!bIsSpectating)
    {
        return;
    }
    
    TArray<APawn*> Targets;
    CollectAliveTargets(Targets);

    if (Targets.Num() == 0)
    {
        return;
    }
    int32 CurrentIndex = Targets.Find(CurrentSpectatorTarget.Get());
    
    // -1인 경우(못 찾음) 처리를 위해 로직 분리
    if (CurrentIndex == -1)
    {
        CurrentIndex = 0;
    }
    
    // 음수 나머지 처리(뒤로 순환)
    int32 NewIndex = (CurrentIndex - 1 + Targets.Num()) % Targets.Num();

    SetSpectatorTarget(Targets[NewIndex]);
}

APawn* USFSpectatorComponent::GetCurrentSpectatorTarget() const
{
    return CurrentSpectatorTarget.Get();
}

void USFSpectatorComponent::CollectAliveTargets(TArray<APawn*>& OutTargets) const
{
    OutTargets.Reset();

    APlayerController* PC = GetController<APlayerController>();
    if (!PC || !PC->GetWorld() || !PC->GetWorld()->GetGameState())
    {
        return;
    }
    
    AGameStateBase* GS = PC->GetWorld()->GetGameState();

    for (APlayerState* PS : GS->PlayerArray)
    {
        // 자신 제외
        if (PS == PC->PlayerState)
        {
            continue;
        }
        // 이미 관전 중인 사람 제외 
        if (PS->IsSpectator())
        {
            continue;
        }
        // ASFPlayerState 사망 여부 확인
        ASFPlayerState* SFPS = Cast<ASFPlayerState>(PS);
        if (SFPS && SFPS->IsDead())
        {
            continue;
        }
        // Pawn 유효성 체크
        if (APawn* Pawn = PS->GetPawn())
        {
            OutTargets.Add(Pawn);
        }
    }

    // PlayerId 기준 정렬 (클라이언트 간 순서 동기화)
    OutTargets.Sort([](const APawn& A, const APawn& B)
        {
        const APlayerState* PSA = A.GetPlayerState();
        const APlayerState* PSB = B.GetPlayerState();
        if (PSA && PSB)
        {
            return PSA->GetPlayerId() < PSB->GetPlayerId();
        }
        return A.GetName() < B.GetName();
    });
}

void USFSpectatorComponent::SetSpectatorTarget(APawn* NewTarget)
{
    APlayerController* PC = GetController<APlayerController>();
    if (!PC || !NewTarget)
    {
        return;
    }
    
    // 이미 보고 있는 대상이면 스킵
    if (CurrentSpectatorTarget == NewTarget)
    {
        return;
    }

    UnbindFromCurrentTarget();
    
    CurrentSpectatorTarget = NewTarget;

    BindToTargetDeathEvent(NewTarget);

    // 서버 알림 (Relevancy)
    Server_SetViewTarget(NewTarget);

    // 로컬 전환 (반응성)
    if (PC->IsLocalController())
    {
        PC->SetViewTargetWithBlend(NewTarget, ViewTargetBlendTime); 
    }

    OnSpectatorTargetChanged.Broadcast(NewTarget);
}

void USFSpectatorComponent::BindToTargetDeathEvent(APawn* Target)
{
    if (!Target)
    {
        return;
    }

    USFPlayerCombatStateComponent* CombatComp = USFPlayerCombatStateComponent::FindPlayerCombatStateComponent(Target);
    if (CombatComp)
    {
        CombatComp->OnCombatInfoChanged.AddDynamic(this, &USFSpectatorComponent::OnSpectatorTargetCombatInfoChanged);
    }
}

void USFSpectatorComponent::UnbindFromCurrentTarget()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(AutoSwitchTimerHandle);
    }
    
    APawn* Target = CurrentSpectatorTarget.Get();
    if (!Target)
    {
        return;
    }

    USFPlayerCombatStateComponent* CombatComp = USFPlayerCombatStateComponent::FindPlayerCombatStateComponent(Target);
    if (CombatComp)
    {
        CombatComp->OnCombatInfoChanged.RemoveDynamic(this, &USFSpectatorComponent::OnSpectatorTargetCombatInfoChanged);
    }
}

void USFSpectatorComponent::OnSpectatorTargetCombatInfoChanged(const FSFHeroCombatInfo& CombatInfo)
{
    if (!bIsSpectating)
    {
        return;
    }

    if (CombatInfo.bIsDead)
    {
        if (GetWorld()->GetTimerManager().IsTimerActive(AutoSwitchTimerHandle))
        {
            return;
        }

        // 약간의 딜레이 후 전환
        GetWorld()->GetTimerManager().SetTimer(
        AutoSwitchTimerHandle,
        this,
        &USFSpectatorComponent::OnAutoSwitchTimerExpired,
        AutoSwitchDelay,
        false);
    }
}

void USFSpectatorComponent::OnAutoSwitchTimerExpired()
{
    if (bIsSpectating)
    {
        SpectateNextPlayer();
    }
}

void USFSpectatorComponent::ShowSpectatorHUD()
{
    APlayerController* PC = GetController<APlayerController>();
    if (!PC || !PC->IsLocalController() || !SpectatorHUDWidgetClass)
    {
        return;
    }

    if (!SpectatorHUDWidget)
    {
        SpectatorHUDWidget = CreateWidget<USFSpectatorHUDWidget>(PC, SpectatorHUDWidgetClass);
    }

    if (SpectatorHUDWidget && !SpectatorHUDWidget->IsInViewport())
    {
        SpectatorHUDWidget->AddToViewport(SpectatorHUDZOrder);
    }
}

void USFSpectatorComponent::HideSpectatorHUD()
{
    if (SpectatorHUDWidget && SpectatorHUDWidget->IsInViewport())
    {
        SpectatorHUDWidget->RemoveFromParent();
    }
}

void USFSpectatorComponent::OnSpectateNextInput(const FInputActionValue& Value)
{
    SpectateNextPlayer();
}

void USFSpectatorComponent::OnSpectatePreviousInput(const FInputActionValue& Value)
{
    SpectatePreviousPlayer();
}

void USFSpectatorComponent::Server_SetViewTarget_Implementation(APawn* NewTarget)
{
    APlayerController* PC = GetController<APlayerController>();
    if (PC)
    {
        // NewTarget이 null이면 컨트롤러 자신(혹은 죽은 Pawn)으로 복귀
        AActor* FinalTarget = NewTarget ? static_cast<AActor*>(NewTarget) : static_cast<AActor*>(PC->GetPawn());
        if (!FinalTarget)
        {
            FinalTarget = PC;
        }
        PC->SetViewTarget(FinalTarget);
    }
}
