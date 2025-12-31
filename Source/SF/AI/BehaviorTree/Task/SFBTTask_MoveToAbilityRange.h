// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "SFBTTask_MoveToAbilityRange.generated.h"

UCLASS()
class SF_API USFBTTask_MoveToAbilityRange : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	USFBTTask_MoveToAbilityRange();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector SelectedAbilityTagKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector DistanceKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector MoveFailCountKey;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float RangeMultiplier = 0.9f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float MaxDuration = 3.0f;

private:
	bool bIsMoving = false;
	float ElapsedTime = 0.0f;
	float CachedAttackRange = 0.0f;
};
