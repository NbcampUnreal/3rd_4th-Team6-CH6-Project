#include "SFPlayFabSubsystem.h"

#include "JsonObjectConverter.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "PlayFabRuntimeSettings.h"

#include "Player/SFPlayerState.h"
#include "System/Data/SFPermanentUpgradeTypes.h"

//===================초기화 및 PlayFab 로그인 실행=======================
void USFPlayFabSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Title ID 설정
	GetMutableDefault<UPlayFabRuntimeSettings>()->TitleId = TEXT("1508CD");

	UE_LOG(LogTemp, Warning, TEXT("[PlayFabSubsystem] Initialized"));
	LoginToPlayFab();
}
//====================================================================

//=========================Subsystem 정리==============================
void USFPlayFabSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RetrySendUpgradeTimerHandle);
	}

	Super::Deinitialize();
}
//====================================================================

//===========================PlayFab 로그인============================
void USFPlayFabSubsystem::LoginToPlayFab()
{
	// OnlineSubsystem 확인 (Steam이어야 함)
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub && OnlineSub->GetSubsystemName() == STEAM_SUBSYSTEM)
	{
		IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();
		if (Identity.IsValid() && Identity->GetLoginStatus(0) == ELoginStatus::LoggedIn)
		{
			// Steam 인증 토큰(세션 티켓) 가져오기
			FString SteamTicket = Identity->GetAuthToken(0);
			if (!SteamTicket.IsEmpty())
			{
				PlayFab::ClientModels::FLoginWithSteamRequest Request;
				Request.TitleId = GetMutableDefault<UPlayFabRuntimeSettings>()->TitleId;
				Request.SteamTicket = SteamTicket;
				Request.CreateAccount = true;
				Request.TicketIsServiceSpecific = false;

				PlayFab::UPlayFabClientAPI::FLoginWithSteamDelegate Success;
				PlayFab::FPlayFabErrorDelegate Error;
				Success.BindUObject(this, &USFPlayFabSubsystem::OnLoginSuccess);
				Error.BindUObject(this, &USFPlayFabSubsystem::OnLoginError);
				PlayFab::UPlayFabClientAPI().LoginWithSteam(Request, Success, Error);
				return;
			}
		}
	}

	UE_LOG(LogTemp, Error, TEXT("[PlayFabSubsystem] Steam login failed or not available"));
}

void USFPlayFabSubsystem::OnLoginSuccess(const PlayFab::ClientModels::FLoginResult& Result)
{
	UE_LOG(LogTemp, Warning, TEXT("[PlayFabSubsystem] Login Success"));
	LoadPlayerData();
}

void USFPlayFabSubsystem::OnLoginError(const PlayFab::FPlayFabCppError& Error)
{
	UE_LOG(LogTemp, Error, TEXT("[PlayFabSubsystem] Login Failed: %s"), *Error.GenerateErrorReport());
}
//====================================================================

//========================세이브 데이터 관리=============================
void USFPlayFabSubsystem::AddGold(int32 Amount)
{
	PlayerStats.Gold += Amount;
	UE_LOG(LogTemp, Warning, TEXT("[PlayFabSubsystem] Gold Updated: %d"), PlayerStats.Gold);
}
//====================================================================

//===========================데이터 저장================================
void USFPlayFabSubsystem::SavePlayerData()
{
	FString JsonString;
	FJsonObjectConverter::UStructToJsonObjectString(PlayerStats, JsonString);

	PlayFab::ClientModels::FUpdateUserDataRequest Request;
	Request.Data.Add(TEXT("SaveJson"), JsonString);

	PlayFab::UPlayFabClientAPI::FUpdateUserDataDelegate Success;
	PlayFab::FPlayFabErrorDelegate Error;

	Success.BindUObject(this, &USFPlayFabSubsystem::OnDataSaved);
	Error.BindUObject(this, &USFPlayFabSubsystem::OnSaveError);

	PlayFab::UPlayFabClientAPI().UpdateUserData(Request, Success, Error);
}

void USFPlayFabSubsystem::OnDataSaved(const PlayFab::ClientModels::FUpdateUserDataResult& Result)
{
	UE_LOG(LogTemp, Warning, TEXT("[PlayFabSubsystem] Save Success"));
}

void USFPlayFabSubsystem::OnSaveError(const PlayFab::FPlayFabCppError& Error)
{
	UE_LOG(LogTemp, Error, TEXT("[PlayFabSubsystem] Save Failed: %s"), *Error.GenerateErrorReport());
}
//====================================================================

//============================데이터 로드===============================
void USFPlayFabSubsystem::LoadPlayerData()
{
	PlayFab::ClientModels::FGetUserDataRequest Request;

	PlayFab::UPlayFabClientAPI::FGetUserDataDelegate Success;
	PlayFab::FPlayFabErrorDelegate Error;

	Success.BindUObject(this, &USFPlayFabSubsystem::OnDataLoaded);
	Error.BindUObject(this, &USFPlayFabSubsystem::OnSaveError);

	PlayFab::UPlayFabClientAPI().GetUserData(Request, Success, Error);
}

void USFPlayFabSubsystem::OnDataLoaded(const PlayFab::ClientModels::FGetUserDataResult& Result)
{
	bHasLoadedPlayerData = true;

	if (const PlayFab::ClientModels::FUserDataRecord* Record = Result.Data.Find(TEXT("SaveJson")))
	{
		FString JsonString = Record->Value;
		UE_LOG(LogTemp, Warning, TEXT("[PlayFabSubsystem] Raw SaveJson: %s"), *JsonString);

		if (!FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &PlayerStats, 0, 0))
		{
			UE_LOG(LogTemp, Error, TEXT("[PlayFabSubsystem] JSON Parse Failed"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[PlayFabSubsystem] No Save Data (use defaults)"));
	}

	// ★ 핵심: PlayFab 로드값은 "클라이언트"에서만 확실히 존재한다.
	// 서버(GameMode 등)에서 GI->Subsystem 값을 읽으면 각 플레이어별 값이 구분되지 않는다.
	StartRetrySendPermanentUpgradeDataToServer();
}
//====================================================================

//========================업그레이드 데이터 서버 전송======================
void USFPlayFabSubsystem::StartRetrySendPermanentUpgradeDataToServer()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (World->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	RetrySendUpgradeAttempts = 0;
	World->GetTimerManager().ClearTimer(RetrySendUpgradeTimerHandle);

	// 즉시 1회 시도 후, 실패하면 타이머로 재시도
	TrySendPermanentUpgradeDataToServer();

	if (!World->GetTimerManager().IsTimerActive(RetrySendUpgradeTimerHandle))
	{
		World->GetTimerManager().SetTimer(
			RetrySendUpgradeTimerHandle,
			this,
			&USFPlayFabSubsystem::TrySendPermanentUpgradeDataToServer,
			0.5f,
			true
		);
	}
}

void USFPlayFabSubsystem::TrySendPermanentUpgradeDataToServer()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (World->GetNetMode() == NM_DedicatedServer)
	{
		World->GetTimerManager().ClearTimer(RetrySendUpgradeTimerHandle);
		return;
	}

	// ✅ 이미 이번 매치에서 전송 성공했으면 추가 전송/재시도 중단
	if (bPermanentUpgradeSent)
	{
		World->GetTimerManager().ClearTimer(RetrySendUpgradeTimerHandle);
		return;
	}

	++RetrySendUpgradeAttempts;
	if (RetrySendUpgradeAttempts > MaxRetrySendUpgradeAttempts)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PlayFabSubsystem] SendUpgradeData aborted - timeout"));
		World->GetTimerManager().ClearTimer(RetrySendUpgradeTimerHandle);
		return;
	}

	// ✅ 로컬 플레이어 기준으로 PC를 확정 (PC0 레이스 제거)
	UGameInstance* GI = World->GetGameInstance();
	if (!GI)
	{
		return;
	}

	ULocalPlayer* LocalPlayer = GI->GetFirstGamePlayer();
	if (!LocalPlayer)
	{
		return;
	}

	APlayerController* PC = LocalPlayer->GetPlayerController(World);
	if (!PC)
	{
		return;
	}

	ASFPlayerState* PS = PC->GetPlayerState<ASFPlayerState>();
	if (!PS)
	{
		return;
	}

	FSFPermanentUpgradeData UpgradeData;
	UpgradeData.Wrath = PlayerStats.Wrath;
	UpgradeData.Pride = PlayerStats.Pride;
	UpgradeData.Lust  = PlayerStats.Lust;
	UpgradeData.Sloth = PlayerStats.Sloth;
	UpgradeData.Greed = PlayerStats.Greed;

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[PlayFabSubsystem] SendUpgradeData | NetMode=%d Wrath=%d Pride=%d Lust=%d Sloth=%d Greed=%d"),
		(int32)World->GetNetMode(),
		UpgradeData.Wrath,
		UpgradeData.Pride,
		UpgradeData.Lust,
		UpgradeData.Sloth,
		UpgradeData.Greed
	);

	// Standalone/ListenServer(로컬)에서는 바로 세팅 가능, Client에서는 Server RPC로 전송
	if (PS->HasAuthority())
	{
		PS->SetPermanentUpgradeData(UpgradeData);
	}
	else
	{
		PS->Server_SubmitPermanentUpgradeData(UpgradeData);
	}

	// ✅ 성공 처리: 이후 타이머 재시도/중복 전송 방지
	bPermanentUpgradeSent = true;
	World->GetTimerManager().ClearTimer(RetrySendUpgradeTimerHandle);
}
//====================================================================
void USFPlayFabSubsystem::ResetPermanentUpgradeSendState()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[PlayFabSubsystem] ResetPermanentUpgradeSendState"));

	RetrySendUpgradeAttempts = 0;
	bPermanentUpgradeSent = false;

	World->GetTimerManager().ClearTimer(RetrySendUpgradeTimerHandle);
}