#include "SFPlayerController.h"

#include "AbilitySystemGlobals.h"
#include "SFPlayerState.h"
#include "AbilitySystem/SFAbilitySystemComponent.h"
#include "Components/SFLoadingCheckComponent.h"
#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "GameModes/SFGameState.h"
#include "GameModes/SFStageManagerComponent.h"
#include "UI/Common/SFSkillSelectionScreen.h"

ASFPlayerController::ASFPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	LoadingCheckComponent = CreateDefaultSubobject<USFLoadingCheckComponent>(TEXT("LoadingCheckComponent"));
}

void ASFPlayerController::BeginPlay()
{
	Super::BeginPlay();
	SetInputMode(FInputModeGameOnly());
	SetShowMouseCursor(false);

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (DefaultMappingContext)
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	if (IsLocalController())
	{
		if (ASFGameState* SFGameState = GetWorld()->GetGameState<ASFGameState>())
		{
			if (USFStageManagerComponent* StageManager = SFGameState->GetStageManager())
			{
				// 이미 클리어된 상태면 즉시 처리
				if (StageManager->IsStageCleared())
				{
					HandleStageCleared(StageManager->GetCurrentStageInfo());
				}
				else
				{
					StageManager->OnStageCleared.AddDynamic(this, &ThisClass::HandleStageCleared);
				}
			}
		}
	}
}

void ASFPlayerController::OnUnPossess()
{
	// Make sure the pawn that is being unpossessed doesn't remain our ASC's avatar actor
	if (APawn* PawnBeingUnpossessed = GetPawn())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(PlayerState))
		{
			if (ASC->GetAvatarActor() == PawnBeingUnpossessed)
			{
				ASC->SetAvatarActor(nullptr);
			}
		}
	}

	Super::OnUnPossess();
}

void ASFPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// 클라이언트에서 PlayerController가 늦게 복제된 경우
	if (GetWorld()->IsNetMode(NM_Client))
	{
		if (ASFPlayerState* SFPS = GetPlayerState<ASFPlayerState>())
		{
			if (USFAbilitySystemComponent* SFASC = SFPS->GetSFAbilitySystemComponent())
			{
				SFASC->RefreshAbilityActorInfo();
				SFASC->TryActivateAbilitiesOnSpawn();
			}
		}
	}
}

void ASFPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// 기존 InputComponent를 향상된 버전(EnhancedInputComponent)으로 Cast
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (InGameMenuAction)
		{
			// BindAction: 이 액션이 시작되면(Started) -> ToggleInGameMenu 함수를 실행
			// ETriggerEvent::Started는 "키를 누르는 순간"
			EnhancedInputComponent->BindAction(InGameMenuAction, ETriggerEvent::Started, this, &ASFPlayerController::ToggleInGameMenu);
		}
	}
}

ASFPlayerState* ASFPlayerController::GetSFPlayerState() const
{
	return CastChecked<ASFPlayerState>(PlayerState, ECastCheckedType::NullAllowed);
}

USFAbilitySystemComponent* ASFPlayerController::GetSFAbilitySystemComponent() const
{
	const ASFPlayerState* LCPS = GetSFPlayerState();
	return (LCPS ? LCPS->GetSFAbilitySystemComponent() : nullptr);
}


void ASFPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (USFAbilitySystemComponent* SFASC = GetSFAbilitySystemComponent())
	{
		SFASC->ProcessAbilityInput(DeltaTime, bGamePaused);
	}
	
	Super::PostProcessInput(DeltaTime, bGamePaused);

}

void ASFPlayerController::ToggleInGameMenu()
{
	UE_LOG(LogTemp, Warning, TEXT("ESC 키 눌림! ToggleInGameMenu 함수 진입 성공"));
	
	// 1. 이미 메뉴가 켜져 있다면? -> 종료
	if (InGameMenuInstance && InGameMenuInstance->IsInViewport())
	{
		InGameMenuInstance->RemoveFromParent();
		InGameMenuInstance = nullptr;

		SetInputMode(FInputModeGameOnly());
		bShowMouseCursor = false;
		return;
	}

	// 2. 메뉴가 꺼져 있다면? -> 실행
	if (InGameMenuClass)
	{
		InGameMenuInstance = CreateWidget<UUserWidget>(this, InGameMenuClass);

		if (InGameMenuInstance)
		{
			// Z-Order 100으로 최상단 배치
			InGameMenuInstance->AddToViewport(100);

			// 위젯 자체를 포커스 타겟으로 명확히 지정
			FInputModeUIOnly InputMode;
			// 위젯이 포커스를 받을 수 있게 설정 
			InputMode.SetWidgetToFocus(InGameMenuInstance->TakeWidget());
			// 마우스가 화면 밖으로 나가지 않게 잠금
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputMode);
			bShowMouseCursor = true;

			InGameMenuInstance->SetKeyboardFocus();
		}
	}
	
}

void ASFPlayerController::HandleStageCleared(const FSFStageInfo& ClearedStageInfo)
{
	if (!ClearedStageInfo.IsBossStage() && !ClearedStageInfo.IsNormalStage())
	{
		return;
	}

	ASFPlayerState* SFPS = GetPlayerState<ASFPlayerState>();
	if (!SFPS)
	{
		return;
	}

	if (!SFPS->IsPawnDataLoaded())
	{
		// 아직 로드 안됨 - 대기
		bPendingStageCleared = true;
		PendingStageInfo = ClearedStageInfo;

		// 기존 핸들 해제 후 새로 바인딩
		if (PawnDataLoadedHandle.IsValid())
		{
			SFPS->OnPawnDataLoaded.Remove(PawnDataLoadedHandle);
		}
  
		// 로드 완료 대기
		PawnDataLoadedHandle = SFPS->OnPawnDataLoaded.AddLambda([this](const USFPawnData* PawnData)
		{
			if (bPendingStageCleared)
			{
				bPendingStageCleared = false;
				ShowSkillSelectionScreen(PendingStageInfo.StageIndex);
			}
		});
			
		return;
	}
	
	ShowSkillSelectionScreen(ClearedStageInfo.StageIndex);
}

void ASFPlayerController::ShowSkillSelectionScreen(int32 StageIndex)
{
	if (SkillSelectionScreenInstance && SkillSelectionScreenInstance->IsInViewport())
	{
		return;
	}

	if (!SkillSelectionScreenClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PlayerController] SkillSelectionScreenClass is not set!"));
		return;
	}

	SkillSelectionScreenInstance = CreateWidget<USFSkillSelectionScreen>(this, SkillSelectionScreenClass);
	if (SkillSelectionScreenInstance)
	{
		// 선택 완료 바인딩
		SkillSelectionScreenInstance->OnSelectionCompleteDelegate.AddDynamic(this, &ThisClass::OnSkillSelectionComplete);

		// 스테이지 정보로 초기화
		SkillSelectionScreenInstance->InitializeSelection(StageIndex);

		// 화면에 표시
		SkillSelectionScreenInstance->AddToViewport(50);

		// UI 모드로 전환
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(SkillSelectionScreenInstance->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
}

void ASFPlayerController::OnSkillSelectionComplete()
{
	HideSkillSelectionScreen();
}


void ASFPlayerController::HideSkillSelectionScreen()
{
	if (SkillSelectionScreenInstance)
	{
		SkillSelectionScreenInstance->OnSelectionCompleteDelegate.RemoveAll(this);
		SkillSelectionScreenInstance->RemoveFromParent();
		SkillSelectionScreenInstance = nullptr;

		// 게임 모드로 복귀
		SetInputMode(FInputModeGameOnly());
		bShowMouseCursor = false;
	}
}
