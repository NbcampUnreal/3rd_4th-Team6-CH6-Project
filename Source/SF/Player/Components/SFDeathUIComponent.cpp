#include "SFDeathUIComponent.h"

#include "SFSpectatorComponent.h"
#include "Components/GameFrameworkComponentDelegates.h"
#include "Player/SFPlayerState.h"
#include "System/SFInitGameplayTags.h"
#include "UI/InGame/SFDeathScreenWidget.h"

const FName USFDeathUIComponent::NAME_DeathUIFeature("DeathUI");

USFDeathUIComponent::USFDeathUIComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
}

void USFDeathUIComponent::OnRegister()
{
	Super::OnRegister();
	RegisterInitStateFeature();
}

void USFDeathUIComponent::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = GetController<APlayerController>();
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	// 초기 상태(Spawned) 진입 시도
	ensure(TryToChangeInitState(SFGameplayTags::InitState_Spawned));
    
	// 초기화 체인 시작
	CheckDefaultInitialization();
}

void USFDeathUIComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (USFPlayerCombatStateComponent* CombatComp = CachedCombatComp.Get())
	{
		CombatComp->OnCombatInfoChanged.RemoveDynamic(this, &ThisClass::OnCombatInfoChanged);
	}
	
	UnregisterInitStateFeature();
	Super::EndPlay(EndPlayReason);
}

void USFDeathUIComponent::CheckDefaultInitialization()
{
	static const TArray<FGameplayTag> StateChain = { SFGameplayTags::InitState_Spawned, SFGameplayTags::InitState_DataAvailable, SFGameplayTags::InitState_DataInitialized, SFGameplayTags::InitState_GameplayReady };

	ContinueInitStateChain(StateChain);
}

bool USFDeathUIComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);

	APlayerController* PC = GetController<APlayerController>();
	if (!PC)
	{
		return false;
	}

	// [None -> Spawned]
	if (!CurrentState.IsValid() && DesiredState == SFGameplayTags::InitState_Spawned)
	{
		return true;
	}

	// [Spawned -> DataAvailable]: PlayerState + CombatInfo 복제 완료 확인
	if (CurrentState == SFGameplayTags::InitState_Spawned && DesiredState == SFGameplayTags::InitState_DataAvailable)
	{
		ASFPlayerState* PS = PC->GetPlayerState<ASFPlayerState>();
		if (!PS)
		{
			return false;
		}

		USFPlayerCombatStateComponent* CombatComp = PS->FindComponentByClass<USFPlayerCombatStateComponent>();
		if (!CombatComp)
		{
			return false;
		}

		//  데이터가 아직 안 왔다면 콜백 등록 후 대기
		if (!CombatComp->HasReceivedInitialCombatInfo())
		{
			USFDeathUIComponent* MutableThis = const_cast<USFDeathUIComponent*>(this);
			if (!CombatComp->OnCombatInfoChanged.IsAlreadyBound(MutableThis, &ThisClass::OnInitialCombatInfoReceived))
			{
				CombatComp->OnCombatInfoChanged.AddDynamic(MutableThis, &ThisClass::OnInitialCombatInfoReceived);
			}
			return false;
		}

		return true;
	}

	// [DataAvailable -> DataInitialized]
	if (CurrentState == SFGameplayTags::InitState_DataAvailable && DesiredState == SFGameplayTags::InitState_DataInitialized)
	{
		return true;
	}

	// [DataInitialized -> GameplayReady]
	if (CurrentState == SFGameplayTags::InitState_DataInitialized && DesiredState == SFGameplayTags::InitState_GameplayReady)
	{
		return true;
	}

	return false;
}

void USFDeathUIComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	if (CurrentState == SFGameplayTags::InitState_DataAvailable && DesiredState == SFGameplayTags::InitState_DataInitialized)
	{
		APlayerController* PC = GetController<APlayerController>();
		ASFPlayerState* PS = PC ? PC->GetPlayerState<ASFPlayerState>() : nullptr;
        
		if (PS)
		{
			USFPlayerCombatStateComponent* CombatComp = PS->GetComponentByClass<USFPlayerCombatStateComponent>();
			InitializeDeathSystem(CombatComp);
		}
	}
}

void USFDeathUIComponent::OnInitialCombatInfoReceived(const FSFHeroCombatInfo& CombatInfo)
{
	// 임시 바인딩 해제
	if (APlayerController* PC = GetController<APlayerController>())
	{
		if (ASFPlayerState* PS = PC->GetPlayerState<ASFPlayerState>())
		{
			if (USFPlayerCombatStateComponent* CombatComp = PS->FindComponentByClass<USFPlayerCombatStateComponent>())
			{
				CombatComp->OnCombatInfoChanged.RemoveDynamic(this, &ThisClass::OnInitialCombatInfoReceived);
			}
		}
	}

	// InitState 재시도 (이제 HasReceivedInitialCombatInfo가 true)
	CheckDefaultInitialization();
}

void USFDeathUIComponent::InitializeDeathSystem(USFPlayerCombatStateComponent* CombatComp)
{
	if (!CombatComp)
	{
		return;
	}

	CachedCombatComp = CombatComp;

	// 이벤트 바인딩
	if (!CombatComp->OnCombatInfoChanged.IsAlreadyBound(this, &ThisClass::OnCombatInfoChanged))
	{
		CombatComp->OnCombatInfoChanged.AddDynamic(this, &ThisClass::OnCombatInfoChanged);
	}

	// 초기 상태 기록 (중복 호출 방지)
	bLastKnownDeadState = CombatComp->IsDead();

	// Seamless Travel 복구: 이미 죽어있으면 바로 관전 모드 (DeathScreen 스킵)
	if (CombatComp->IsDead())
	{
		APlayerController* PC = GetController<APlayerController>();
		if (PC)
		{
			if (USFSpectatorComponent* SpectatorComp = PC->FindComponentByClass<USFSpectatorComponent>())
			{
				SpectatorComp->StartSpectating();
			}
		}
	}
}

void USFDeathUIComponent::OnCombatInfoChanged(const FSFHeroCombatInfo& CombatInfo)
{
	// 상태 변화 없으면 무시 (중복 호출 방지)
	if (CombatInfo.bIsDead == bLastKnownDeadState)
	{
		return;
	}

	bLastKnownDeadState = CombatInfo.bIsDead;

	if (CombatInfo.bIsDead)
	{
		// 게임 플레이 중 사망 → DeathScreen 표시
		ShowDeathScreen();
	}
	else
	{
		// 부활
		HideDeathScreen();
	}
}

void USFDeathUIComponent::ShowDeathScreen()
{
	APlayerController* PC = GetController<APlayerController>();
	if (!PC || !DeathScreenWidgetClass)
	{
		return;
	}

	if (!DeathScreenWidget)
	{
		DeathScreenWidget = CreateWidget<USFDeathScreenWidget>(PC, DeathScreenWidgetClass);
		DeathScreenWidget->OnDeathAnimationFinished.AddDynamic(this, &ThisClass::OnDeathAnimationFinished);
	}

	if (DeathScreenWidget && !DeathScreenWidget->IsInViewport())
	{
		DeathScreenWidget->AddToViewport(DeathScreenZOrder);
		DeathScreenWidget->PlayDeathDirection();
	}
}

void USFDeathUIComponent::HideDeathScreen()
{
	APlayerController* PC = GetController<APlayerController>();
	if (!PC)
	{
		return;
	}

	// 관전 모드 종료
	if (USFSpectatorComponent* SpectatorComp = PC->FindComponentByClass<USFSpectatorComponent>())
	{
		SpectatorComp->StopSpectating();
	}

	// DeathScreen 제거
	if (DeathScreenWidget && DeathScreenWidget->IsInViewport())
	{
		DeathScreenWidget->RemoveFromParent();
	}
}

void USFDeathUIComponent::OnDeathAnimationFinished()
{
	if (DeathScreenWidget && DeathScreenWidget->IsInViewport())
	{
		DeathScreenWidget->RemoveFromParent();
	}

	if (USFPlayerCombatStateComponent* CombatComp = CachedCombatComp.Get())
	{
		if (!CombatComp->IsDead())
		{
			return; 
		}
	}

	// 관전 모드 시작
	APlayerController* PC = GetController<APlayerController>();
	if (PC)
	{
		if (USFSpectatorComponent* SpectatorComp = PC->FindComponentByClass<USFSpectatorComponent>())
		{
			SpectatorComp->StartSpectating();
		}
	}
}