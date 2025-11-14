// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "SFGameState.generated.h"

class USFPortalManagerComponent;
class ASFPortal;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPortalActivated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPortalPlayerCountChanged, int32, CurrentCount, int32, RequiredCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPortalPlayerEntered, const FSFPlayerSelectionInfo&, PlayerInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPortalPlayerLeft, const FSFPlayerSelectionInfo&, PlayerInfo);

/**
 * 
 */
UCLASS()
class SF_API ASFGameState : public AGameState
{
	GENERATED_BODY()

	public:
	ASFGameState();
	
	/** Portal Manager 가져오기 */
	UFUNCTION(BlueprintPure, Category = "SF|GameState")
	USFPortalManagerComponent* GetPortalManager() const { return PortalManager; }

private:
	/** Portal 관리 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SF|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USFPortalManagerComponent> PortalManager;
};
