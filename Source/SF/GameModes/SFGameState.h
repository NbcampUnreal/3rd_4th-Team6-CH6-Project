// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "SFGameState.generated.h"

class USFPortalManagerComponent;
class ASFPortal;

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
