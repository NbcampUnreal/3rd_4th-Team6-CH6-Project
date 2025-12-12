#include "SFStageSubsystem.h"

#include "SFLogChannels.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

void USFStageSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    CurrentStageInfo = FSFStageInfo();

    // DataTable 비동기 로드
    if (!StageConfigTable.IsNull())
    {
        FStreamableDelegate OnLoaded = FStreamableDelegate::CreateUObject(
            this, &USFStageSubsystem::OnConfigTableLoaded);
        
        ConfigTableLoadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
            StageConfigTable.ToSoftObjectPath(),
            OnLoaded,
            FStreamableManager::AsyncLoadHighPriority
        );
    }
    
    // Hard Travel 감지
    FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &USFStageSubsystem::OnPreLoadMap);
    FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &USFStageSubsystem::OnPostLoadMapWithWorld);

    // Seamless Travel 감지
    FWorldDelegates::OnSeamlessTravelStart.AddUObject(this, &USFStageSubsystem::OnSeamlessTravelStart);
}

void USFStageSubsystem::Deinitialize()
{
    FCoreUObjectDelegates::PreLoadMap.RemoveAll(this);
    FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
    FWorldDelegates::OnSeamlessTravelStart.RemoveAll(this);

    ConfigTableLoadHandle.Reset();
    
    Super::Deinitialize();
}

bool USFStageSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    // 서버와 클라이언트 모두에서 생성
    return true;
}

void USFStageSubsystem::OnPreLoadMap(const FString& MapName)
{
    UpdateStageInfoFromLevel(MapName);
}

void USFStageSubsystem::OnSeamlessTravelStart(UWorld* CurrentWorld, const FString& LevelName)
{
    UpdateStageInfoFromLevel(LevelName);
}

void USFStageSubsystem::OnPostLoadMapWithWorld(UWorld* LoadedWorld)
{
   
}

void USFStageSubsystem::SetCurrentStageInfo(const FSFStageInfo& NewStageInfo)
{
    if (CurrentStageInfo != NewStageInfo)
    {
        CurrentStageInfo = NewStageInfo;
    }
}

void USFStageSubsystem::ResetStageInfo()
{
    CurrentStageInfo = FSFStageInfo();
}

FSFStageInfo USFStageSubsystem::GetStageInfoForLevel(const FString& LevelName) const
{
    if (const FSFStageConfig* Config = GetStageConfigForLevel(LevelName))
    {
        return Config->StageInfo;
    }
    return FSFStageInfo();

}

const FSFStageConfig* USFStageSubsystem::GetStageConfigForLevel(const FString& LevelName) const
{
    UDataTable* ConfigTable = CachedConfigTable;
    
    // 아직 로드 안됐으면 동기 로드 (폴백)
    if (!ConfigTable)
    {
        ConfigTable = StageConfigTable.LoadSynchronous();
    }
    
    if (!ConfigTable)
    {
        UE_LOG(LogSF, Warning, TEXT("[StageSubsystem] StageConfigTable is not set!"));
        return nullptr;
    }

    FString ShortName = FPackageName::GetShortName(LevelName);
    return ConfigTable->FindRow<FSFStageConfig>(FName(*ShortName), TEXT("GetStageConfigForLevel"));
}

void USFStageSubsystem::UpdateStageInfoFromLevel(const FString& LevelName)
{
    if (const FSFStageConfig* Config = GetStageConfigForLevel(LevelName))
    {
        SetCurrentStageInfo(Config->StageInfo);
    }
}

void USFStageSubsystem::OnConfigTableLoaded()
{
    CachedConfigTable = StageConfigTable.Get();

    if (CachedConfigTable)
    {
        UE_LOG(LogSF, Log, TEXT("[StageSubsystem] Config table loaded successfully"));
    }
    else
    {
        UE_LOG(LogSF, Error, TEXT("[StageSubsystem] Failed to load config table!"));
    }
    
    ConfigTableLoadHandle.Reset();
}
