#include "SFLoadingScreenSubsystem.h"
#include "MoviePlayer.h"
#include "Framework/Application/SlateApplication.h"
#include "GameMapsSettings.h"
#include "Blueprint/UserWidget.h"
#include "Engine/GameViewportClient.h"
#include "UI/Slate/SFSimpleLoadingScreen.h"

#include "HAL/IConsoleManager.h"

void USFLoadingScreenSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
    
    // 1. 하드 트래블(OpenLevel) 감지
    FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &USFLoadingScreenSubsystem::OnPreLoadMap);

    // 2. 맵 로드 완료 감지 
    FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &USFLoadingScreenSubsystem::OnPostLoadMapWithWorld);
    
    bCurrentLoadingScreenStarted = false;
}

void USFLoadingScreenSubsystem::Deinitialize()
{
    RemoveWidgetFromViewport();
    
    FCoreUObjectDelegates::PreLoadMap.RemoveAll(this);
    FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
	Super::Deinitialize();
}

bool USFLoadingScreenSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    const UGameInstance* GameInstance = CastChecked<UGameInstance>(Outer);
    const bool bIsServerWorld = GameInstance->IsDedicatedServerInstance();
    return !bIsServerWorld;
}

void USFLoadingScreenSubsystem::SetPreLoadingScreenContentWidget(TSubclassOf<UUserWidget> NewWidgetClass)
{
    if (PreLoadingScreenWidgetClass != NewWidgetClass)
    {
        PreLoadingScreenWidgetClass = NewWidgetClass;
        OnPreLoadingScreenWidgetChanged.Broadcast(PreLoadingScreenWidgetClass);
    }
}

TSubclassOf<UUserWidget> USFLoadingScreenSubsystem::GetPreLoadingScreenContentWidget() const
{
    return PreLoadingScreenWidgetClass;
}

void USFLoadingScreenSubsystem::SetLoadingScreenContentWidget(TSubclassOf<UUserWidget> NewWidgetClass)
{
    if (LoadingScreenWidgetClass != NewWidgetClass)
    {
        LoadingScreenWidgetClass = NewWidgetClass;
        OnLoadingScreenWidgetChanged.Broadcast(LoadingScreenWidgetClass);
    }
}

TSubclassOf<UUserWidget> USFLoadingScreenSubsystem::GetLoadingScreenContentWidget() const
{
    return LoadingScreenWidgetClass;
}

void USFLoadingScreenSubsystem::OnPreLoadMap(const FString& MapName)
{
    StartLoadingScreen();
}

void USFLoadingScreenSubsystem::StartLoadingScreen()
{
    if (bCurrentLoadingScreenStarted)
    {
        return;
    }
    
    if (GetMoviePlayer()->IsMovieCurrentlyPlaying())
    {
        return; 
    }

    FLoadingScreenAttributes LoadingScreen;
    UGameInstance* LocalGameInstance = GetGameInstance();
    TSubclassOf<UUserWidget> LoadedWidgetClass = PreLoadingScreenWidget.TryLoadClass<UUserWidget>();
    if (UUserWidget* LoadingWidget = UUserWidget::CreateWidgetInstance(*LocalGameInstance, LoadedWidgetClass, NAME_None))
    {
        PreLoadingSWidgetPtr = LoadingWidget->TakeWidget();
        LoadingScreen.WidgetLoadingScreen = PreLoadingSWidgetPtr;
    }
    else
    {
        LoadingScreen.WidgetLoadingScreen = SNew(SSFSimpleLoadingScreen); 
    }
    
    // 하드 트래블 최적화
    LoadingScreen.bAllowEngineTick = false; 
    LoadingScreen.bWaitForManualStop = false;
    LoadingScreen.bAutoCompleteWhenLoadingCompletes = true;
    LoadingScreen.bMoviesAreSkippable = false;
    LoadingScreen.MinimumLoadingScreenDisplayTime = 3.0f; // 최소 로딩 스크린 표시 시간
    LoadingScreen.PlaybackType = MT_Normal;

    GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
    GetMoviePlayer()->PlayMovie();
    
    // 3D 렌더링 비활성화 (플리커링 방지)
    if (UGameViewportClient* GameViewport = GetWorld()->GetGameViewport())
    {
       GameViewport->bDisableWorldRendering = true;
    }

    bCurrentLoadingScreenStarted = true;
}

void USFLoadingScreenSubsystem::OnPostLoadMapWithWorld(UWorld* LoadedWorld)
{
    if (bCurrentLoadingScreenStarted)
    {
        // 하드 트래블 완료 (bAutoCompleteWhenLoadingCompletes=true라면 엔진이 알아서 끄지만 명시적으로 호출)
        StopLoadingScreen();
    }
}

void USFLoadingScreenSubsystem::StopLoadingScreen()
{
    GetMoviePlayer()->StopMovie();
    
    if (UGameViewportClient* GameViewport = GetWorld()->GetGameViewport())
    {
        GameViewport->bDisableWorldRendering = false;
    }
    
    RemoveWidgetFromViewport();
    bCurrentLoadingScreenStarted = false;
}

void USFLoadingScreenSubsystem::RemoveWidgetFromViewport()
{
    UGameInstance* LocalGameInstance = GetGameInstance();
    if (PreLoadingSWidgetPtr.IsValid())
    {
        if (UGameViewportClient* GameViewportClient = LocalGameInstance->GetGameViewportClient())
        {
            GameViewportClient->RemoveViewportWidgetContent(PreLoadingSWidgetPtr.ToSharedRef());
        }
        PreLoadingSWidgetPtr.Reset();
    }
}

bool USFLoadingScreenSubsystem::IsInSeamlessTravel()
{
    if (UWorld* World = GetWorld())
    {
        if (GEngine)
        {
            // 현재 월드 컨텍스트의 트래블 핸들러 상태 확인
            FSeamlessTravelHandler& TravelHandler = GEngine->SeamlessTravelHandlerForWorld(World);
            return TravelHandler.IsInTransition();
        }
    }
    return false;
}

bool USFLoadingScreenSubsystem::IsTransitionMap(const FString& MapName)
{
    if (UGameMapsSettings* GameMapsSettings = UGameMapsSettings::GetGameMapsSettings())
    {
        // 프로젝트 세팅에 설정된 트랜지션 맵 이름과 비교
        const FString TransitionMapName = UGameMapsSettings::GetGameMapsSettings()->TransitionMap.GetLongPackageName();
    
        // 경로 포함 비교 또는 이름만 비교
        return MapName.Contains(TransitionMapName) || TransitionMapName.Contains(MapName);
    }
    return false;
}