#include "SFGameInstance.h"

#include "GenericTeamAgentInterface.h"
#include "NetworkMessage.h"
#include "SFInitGameplayTags.h"
#include "SFStageSubsystem.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Team/SFTeamTypes.h"

void USFGameInstance::Init()
{
	Super::Init();

	// InitState를 GFCM에 등록
	UGameFrameworkComponentManager* ComponentManager = GetSubsystem<UGameFrameworkComponentManager>(this);

	if (ensure(ComponentManager))
	{
		ComponentManager->RegisterInitState(SFGameplayTags::InitState_Spawned, false, FGameplayTag());
		ComponentManager->RegisterInitState(SFGameplayTags::InitState_DataAvailable, false, SFGameplayTags::InitState_Spawned);
		ComponentManager->RegisterInitState(SFGameplayTags::InitState_DataInitialized, false, SFGameplayTags::InitState_DataAvailable);
		ComponentManager->RegisterInitState(SFGameplayTags::InitState_GameplayReady, false, SFGameplayTags::InitState_DataInitialized);
	}

	InitTeamAttitudeSolver();
	LoadEnemyDataTable();
}

void USFGameInstance::StartMatch()
{
	if (GetWorld()->GetNetMode() == ENetMode::NM_DedicatedServer || GetWorld()->GetNetMode() == ENetMode::NM_ListenServer)
	{
		LoadLevelAndListen(GameLevel);
	}
}

void USFGameInstance::LoadLevelAndListen(TSoftObjectPtr<UWorld> Level)
{
	const FName LevelURL = FName(*FPackageName::ObjectPathToPackageName(Level.ToString()));

	if (USFStageSubsystem* StageSubsystem = GetSubsystem<USFStageSubsystem>())
	{
		if (UWorld* World = GetWorld())
		{
			if (AGameStateBase* GS = World->GetGameState<AGameStateBase>())
			{
				StageSubsystem->SetPlayerCount(GS->PlayerArray.Num());
			}
		}
	}
	
	if (LevelURL != "")
	{
		GetWorld()->ServerTravel(LevelURL.ToString() + "?listen", false);
		//GetWorld()->ServerTravel(LevelURL.ToString(), true);
	}
}

void USFGameInstance::InitTeamAttitudeSolver()
{
	// Player vs Enemy = Hostile
	FGenericTeamId::SetAttitudeSolver([](FGenericTeamId A, FGenericTeamId B) -> ETeamAttitude::Type
	{
		if (A == B)
		{
			return ETeamAttitude::Friendly;
		}
		
		// Player(0) vs Enemy(1) = Hostile
		if ((A == FGenericTeamId(SFTeamID::Player) && B == FGenericTeamId(SFTeamID::Enemy)) ||
			(A == FGenericTeamId(SFTeamID::Enemy) && B == FGenericTeamId(SFTeamID::Player)))
		{
			return ETeamAttitude::Hostile;
		}
        
		return ETeamAttitude::Neutral;
	});
}

void USFGameInstance::LoadEnemyDataTable()
{
    // 1. Enemy Attribute 데이터 로드
    if (EnemyDataTable)
    {
        EnemyDataMap.Empty();
        
        TArray<FName> RowNames = EnemyDataTable->GetRowNames();
        for (const FName& RowName : RowNames)
        {
            if (FEnemyAttributeData* RowData = EnemyDataTable->FindRow<FEnemyAttributeData>(RowName, TEXT("")))
            {
                EnemyDataMap.Add(RowName, *RowData);
            }
        }
        
     
    }

    // 2. Enemy Ability 데이터 로드 
    if (EnemyAbilityDataTable)
    {
        EnemyAbilityMap.Empty();

        TArray<FName> RowNames = EnemyAbilityDataTable->GetRowNames();

        for (const FName& RowName : RowNames)
        {
            // Attack 타입으로 먼저 시도 
            if (FEnemyAttackAbilityData* AttackData = EnemyAbilityDataTable->FindRow<FEnemyAttackAbilityData>(RowName, TEXT("")))
            {
                // AbilityType이 Attack인지 확인
                if (AttackData->AbilityType == EAbilityType::Attack)
                {
                    FAbilityDataWrapper Wrapper(*AttackData);
                    EnemyAbilityMap.Add(RowName, Wrapper);
                    continue;
                }
            }

            // 추후 다른 타입 추가 시 여기에 추가
            // if (FEnemyDefensiveAbilityData* DefensiveData = ...)
            // {
            //     if (DefensiveData->AbilityType == EAbilityType::Defensive)
            //     {
            //         FAbilityDataWrapper Wrapper(*DefensiveData);
            //         EnemyAbilityMap.Add(RowName, Wrapper);
            //         continue;
            //     }
            // }
        }
    }
}
const FAbilityBaseData* USFGameInstance::FindAbilityData(FName AbilityID) const
{
	if (const FAbilityDataWrapper* Wrapper = EnemyAbilityMap.Find(AbilityID))
	{
		return Wrapper->GetBaseData();
	}

	return nullptr;
}

FAbilityDataWrapper* USFGameInstance::FindAbilityDataWrapper(FName AbilityID)
{
	return EnemyAbilityMap.Find(AbilityID);
}

void USFGameInstance::PlayBGM(USoundBase* NewBGM, float FadeDuration)
{
	// 1. 안전 장치: BGM 파일이 없으면 중단
	if (!NewBGM)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayBGM: NewBGM is NULL"));
		StopBGM(FadeDuration);
		return;
	}

	// 2. 컴포넌트가 없으면 생성 (지연 생성)
	// bPersistAcrossLevelTransition을 true -> 맵 이동 시 파괴방지 (내부적으로 처리됨)
	if (!MainAudioComponent)
	{
		MainAudioComponent = UGameplayStatics::CreateSound2D(this, NewBGM, 1.0f,
			1.0f,0.0f,nullptr, true, false);

		if (MainAudioComponent)
		{
			MainAudioComponent->bAutoDestroy = false; // 소리 끝났다고 컴포넌트 삭제되지 않게 설정
		}
		else
		{
			return;
		}
	}
	
	// 3. 이미 같은 노래가 나오고 있으면 무시
	if (CurrentBGMSound == NewBGM && MainAudioComponent->IsPlaying())
	{
		
		return;
	}

	// 4. 재생 로직
	CurrentBGMSound = NewBGM;

	// 만약 이미 재생 중이면 페이드 아웃 후 교체 (자연스럽게)
	if (MainAudioComponent->IsPlaying())
	{
		// 기존 BGM 페이드 아웃 
		MainAudioComponent->SetSound(NewBGM);
		MainAudioComponent->FadeIn(FadeDuration, 1.0f);
	}
	else
	{
		// 멈춰있던 상태면 그냥 재생
		MainAudioComponent->SetSound(NewBGM);
		MainAudioComponent->FadeIn(FadeDuration, 1.0f);
	}
}

void USFGameInstance::StopBGM(float FadeDuration)
{
	if (MainAudioComponent && MainAudioComponent->IsPlaying())
	{
		MainAudioComponent->FadeOut(FadeDuration, 0.0f);
		CurrentBGMSound = nullptr;
	}
}

void USFGameInstance::SetCombatState(bool bInCombat)
{
	return;
}
