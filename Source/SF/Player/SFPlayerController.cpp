#include "SFPlayerController.h"

#include "AbilitySystemGlobals.h"
#include "SFPlayerState.h"
#include "AbilitySystem/SFAbilitySystemComponent.h"
#include "Components/SFLoadingCheckComponent.h"
#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "GameModes/SFGameState.h"
#include "GameModes/SFStageManagerComponent.h"
#include "UI/Common/SFSkillSelectionScreen.h"
#include "UI/InGame/SFIndicatorWidgetBase.h"

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

	// 로컬 플레이어인 경우 팀원 표시 로직 실행
	if (IsLocalController())
	{
		GetWorld()->GetTimerManager().SetTimer(
			TeammateSearchTimerHandle,
			this,
			&ASFPlayerController::CreateTeammateIndicators,
			1.0f,
			true
			);
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

void ASFPlayerController::CreateTeammateIndicators()
{
	if (!TeammateIndicatorWidgetClass) 
	{
		return;
	}

	// 1. 죽거나 사라진 팀원 위젯 청소 (Cleanup)
	for (auto It = TeammateWidgetMap.CreateIterator(); It; ++It)
	{
		AActor* KeyActor = It.Key();
		USFIndicatorWidgetBase* Widget = It.Value();

		// 조건: 액터가 파괴되었거나(IsValid 실패), 위젯이 타겟을 잃어버렸다면
		if (!IsValid(KeyActor) || (Widget && !Widget->HasValidTarget()))
		{
			if (Widget)
			{
				Widget->RemoveFromParent(); // 화면에서 즉시 지움
			}
			It.RemoveCurrent();
		}
	}

	// 2. 새로운 팀원 검색 및 위젯 생성

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), FoundActors);

	APawn* MyPawn = GetPawn();

	for (AActor* Actor : FoundActors)
	{
		// A. 플레이어 자신이면 패스
		if (Actor == MyPawn) continue;

		// B. 이미 위젯을 생성한 액터면 패스
		if (TeammateWidgetMap.Contains(Actor)) continue;

		// C. 진짜 플레이어인지 확인 (몬스터 제외)
		APawn* TargetPawn = Cast<APawn>(Actor);
		if (!TargetPawn) continue;

		// PlayerState가 없으면 봇이나 몬스터로 간주하고 패스 -> 혹시 PlayerState가 늦게 로딩될 수도 있으니, 이번 틱에 없으면 다음 틱에 재검사
		if (TargetPawn->GetPlayerState() == nullptr) continue;

		// === [조건 만족: 위젯 생성] ===
		USFIndicatorWidgetBase* NewIndicator = CreateWidget<USFIndicatorWidgetBase>(this, TeammateIndicatorWidgetClass);
		if (NewIndicator)
		{
			// 타겟 설정
			NewIndicator->SetTargetActor(Actor);
			
			// 화면 최하단(-1)에 부착 -> 다른 HUD를 가리지 않도록
			NewIndicator->AddToViewport(-1);
			
			// 관리 맵에 등록
			TeammateWidgetMap.Add(Actor, NewIndicator);
			
			UE_LOG(LogTemp, Log, TEXT("Team Indicator Created for: %s"), *Actor->GetName());
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
