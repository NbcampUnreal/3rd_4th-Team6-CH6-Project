// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "SFBTS_AutoFaceTarget.generated.h"

/**
 * 
 */
UCLASS()
class SF_API USFBTS_AutoFaceTarget : public UBTService
{
	GENERATED_BODY()

public:
	USFBTS_AutoFaceTarget();

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	
	UPROPERTY(EditAnywhere, Category = "Target")
	FBlackboardKeySelector TargetKey;

	
	UPROPERTY(EditAnywhere, Category = "Settings")
	float AcceptableAngle = 10.0f;

	
	UPROPERTY(EditAnywhere, Category = "Settings")
	float MovementSpeedThreshold = 10.0f;

	
	UPROPERTY(EditAnywhere, Category = "Settings")
	bool bSkipDuringAbility = true;
};
