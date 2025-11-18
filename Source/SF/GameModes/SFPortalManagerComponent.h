#pragma once

#include "CoreMinimal.h"
#include "Components/GameStateComponent.h"
#include "Player/SFPlayerInfoTypes.h"
#include "SFPortalManagerComponent.generated.h"

struct FSFPlayerSelectionInfo;
class ASFPortal;

/**
 * 포탈에서 각 플레이어의 상태
 */
USTRUCT(BlueprintType)
struct FPortalPlayerStatus
{
    GENERATED_BODY()
    
    // PlayerState 참조 (UI에서 직접 SelectionInfo 조회용)
    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<APlayerState> PlayerState = nullptr;
    
    // 포탈에 진입했는지 (Ready)
    UPROPERTY(BlueprintReadOnly)
    bool bIsInPortal = false;
    
    FPortalPlayerStatus() = default;
    
    FPortalPlayerStatus(APlayerState* InPS, bool bInPortal)
         : PlayerState(InPS)
         , bIsInPortal(bInPortal)
    {}
};

/**
 * 포탈의 모든 상태를 포함하는 단일 메시지
 */
USTRUCT(BlueprintType)
struct FSFPortalStateMessage
{
    GENERATED_BODY()
    
    // 포탈 활성화 여부
    UPROPERTY(BlueprintReadOnly)
    bool bIsActive = false;
    
    // 포탈에 진입한 플레이어들 
    UPROPERTY(BlueprintReadOnly)
    TArray<TObjectPtr<APlayerState>> PlayersInPortal;
    
    // 전체 플레이어 수
    UPROPERTY(BlueprintReadOnly)
    int32 TotalPlayerCount = 0;

    // Travel 대기 시간 (카운트다운 표시용, -1이면 대기중 아님)
    UPROPERTY(BlueprintReadOnly)
    float TravelCountdown = -1.0f;
    
    // Travel 준비 완료 여부 (모든 플레이어 진입)
    UPROPERTY(BlueprintReadOnly)
    bool bReadyToTravel = false;
};

/**
 * Portal 시스템을 관리하는 GameState Component
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SF_API USFPortalManagerComponent : public UGameStateComponent
{
	GENERATED_BODY()

public:
    USFPortalManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
    
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void BeginPlay() override;

    /** Portal 활성화 (스테이지 클리어 시) */
    UFUNCTION(BlueprintCallable, Category = "SF|Portal", BlueprintAuthorityOnly)
    void ActivatePortal();
    
    /** Portal이 활성화되었는지 */
    UFUNCTION(BlueprintPure, Category = "SF|Portal")
    bool IsPortalActive() const { return bPortalActive; }

    /** 플레이어가 Portal에 진입 */
    UFUNCTION(BlueprintCallable, Category = "SF|Portal")
    void NotifyPlayerEnteredPortal(APlayerState* PlayerState);
    
    /** 플레이어가 Portal에서 이탈 */
    UFUNCTION(BlueprintCallable, Category = "SF|Portal")
    void NotifyPlayerLeftPortal(APlayerState* PlayerState);
    
    /** Portal Actor 등록 (World에 배치된 Portal이 BeginPlay에서 호출) */
    void RegisterPortal(ASFPortal* Portal);
    
    /** Portal Actor 등록 해제 */
    void UnregisterPortal(ASFPortal* Portal);

    /** 관리 중인 Portal 가져오기 */
    UFUNCTION(BlueprintPure, Category = "SF|Portal")
    ASFPortal* GetManagedPortal() const { return ManagedPortal; }

private:
    UFUNCTION()
    void OnRep_PortalActive();
    
    /** 포탈 상태 메시지 브로드캐스트 */
    void BroadcastPortalState();
    
    /** Portal 준비 체크 및 Travel 시작 */
    void CheckPortalReadyAndTravel();
    
    /** Travel 실행 (타이머 콜백) */
    void ExecuteTravel();

    /** 필요한 플레이어 수 계산 */
    int32 GetRequiredPlayerCount() const;
    
    /** UniqueId로 PlayerState 찾기 */
    APlayerState* FindPlayerStateByUniqueId(const FUniqueNetIdRepl& UniqueId) const;

private:
    /** Portal 활성화 상태 */
    UPROPERTY(ReplicatedUsing = OnRep_PortalActive)
    bool bPortalActive;

    /** Portal에 진입한 플레이어들 (UniqueId) */
    TSet<FUniqueNetIdRepl> PlayersInsidePortal;
    
    /** Travel 대기 타이머 */
    FTimerHandle TravelTimerHandle;
    
    /** Travel 대기 시간 */
    UPROPERTY(EditDefaultsOnly, Category = "SF|Portal")
    float TravelDelayTime = 3.0f;
    
    /** Travel 카운트다운 남은 시간 */
    UPROPERTY()
    float TravelCountdownRemaining = -1.0f;
    
    /** Travel 중인지 */
    bool bIsTraveling;

    /** 현재 관리중인 Portal */
    UPROPERTY()
    TObjectPtr<ASFPortal> ManagedPortal;
};
