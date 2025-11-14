#pragma once

#include "CoreMinimal.h"
#include "Components/GameStateComponent.h"
#include "SFPortalManagerComponent.generated.h"

struct FSFPlayerSelectionInfo;
class ASFPortal;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPortalActivated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPortalPlayerCountChanged, int32, CurrentCount, int32, RequiredCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPortalPlayerEntered, const FSFPlayerSelectionInfo&, PlayerInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPortalPlayerLeft, const FSFPlayerSelectionInfo&, PlayerInfo);

/**
 * Portal 시스템을 관리하는 GameState Component
 * - Portal 활성화 상태 관리
 * - Portal 내 플레이어 추적
 * - Travel 조건 체크
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SF_API USFPortalManagerComponent : public UGameStateComponent
{
	GENERATED_BODY()

public:
	USFPortalManagerComponent();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

	// ========== Portal 활성화 ==========
	
	/** Portal 활성화 (스테이지 클리어 시) */
	UFUNCTION(BlueprintCallable, Category = "SF|Portal", BlueprintAuthorityOnly)
	void ActivatePortal();
	
	/** Portal 비활성화 */
	UFUNCTION(BlueprintCallable, Category = "SF|Portal", BlueprintAuthorityOnly)
	void DeactivatePortal();
	
	/** Portal이 활성화되었는지 */
	UFUNCTION(BlueprintPure, Category = "SF|Portal")
	bool IsPortalActive() const { return bPortalActive; }

	// ========== 플레이어 관리 ==========
	
	/** 플레이어가 Portal에 진입 */
	UFUNCTION(BlueprintCallable, Category = "SF|Portal")
	void NotifyPlayerEnteredPortal(APlayerState* PlayerState);
	
	/** 플레이어가 Portal에서 이탈 */
	UFUNCTION(BlueprintCallable, Category = "SF|Portal")
	void NotifyPlayerLeftPortal(APlayerState* PlayerState);
	
	/** Portal에 있는 플레이어 목록 */
	UFUNCTION(BlueprintPure, Category = "SF|Portal")
	const TArray<FSFPlayerSelectionInfo>& GetPlayersInPortal() const { return PlayersInPortal; }
	
	/** 현재 Portal에 있는 플레이어 수 */
	UFUNCTION(BlueprintPure, Category = "SF|Portal")
	int32 GetCurrentPlayerCount() const { return PlayersInPortal.Num(); }
	
	/** Travel에 필요한 플레이어 수 */
	UFUNCTION(BlueprintPure, Category = "SF|Portal")
	int32 GetRequiredPlayerCount() const;
	
	/** 모든 플레이어가 Portal에 입장했는지 */
	UFUNCTION(BlueprintPure, Category = "SF|Portal")
	bool AreAllPlayersInPortal() const;

	// ========== Portal Actor 관리 ==========
	
	/** Portal Actor 등록 (Portal이 BeginPlay에서 호출) */
	void RegisterPortal(ASFPortal* Portal);
	
	/** Portal Actor 등록 해제 */
	void UnregisterPortal(ASFPortal* Portal);

private:
	UFUNCTION()
	void OnRep_PortalActive();
	
	UFUNCTION()
	void OnRep_PlayersInPortal();
	
	void BroadcastPlayerCountChanged();
	
	/** Portal 준비 체크 및 Travel 시작 */
	void CheckPortalReadyAndTravel();
	
	/** Travel 실행 (타이머 콜백) */
	void ExecuteTravel();
	
	FSFPlayerSelectionInfo GetPlayerSelectionInfo(const APlayerState* PlayerState) const;
	
	/** 등록된 Portal들의 비주얼 업데이트 */
	void UpdatePortalVisuals(bool bEnabled);

public:
	/** Portal 활성화 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "SF|Portal")
	FOnPortalActivated OnPortalActivated;

	/** 플레이어 카운트 변경 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "SF|Portal")
	FOnPortalPlayerCountChanged OnPortalPlayerCountChanged;

	/** 플레이어 입장 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "SF|Portal")
	FOnPortalPlayerEntered OnPortalPlayerEntered;

	/** 플레이어 퇴장 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "SF|Portal")
	FOnPortalPlayerLeft OnPortalPlayerLeft;

private:
	/** Portal 활성화 상태 */
	UPROPERTY(ReplicatedUsing = OnRep_PortalActive)
	bool bPortalActive;
	
	/** Portal에 있는 플레이어 목록 */
	UPROPERTY(ReplicatedUsing = OnRep_PlayersInPortal)
	TArray<FSFPlayerSelectionInfo> PlayersInPortal;
	
	/** Travel 대기 타이머 */
	FTimerHandle TravelTimerHandle;
	
	/** Travel 대기 시간 */
	UPROPERTY(EditDefaultsOnly, Category = "SF|Portal")
	float TravelDelayTime = 3.0f;
	
	/** 이미 Travel 중인지 */
	bool bIsTraveling;
	
	/** 등록된 Portal Actor들 */
	UPROPERTY()
	TArray<TObjectPtr<ASFPortal>> RegisteredPortals;
};
