// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BehaviorTree/Decorators/BTD_IsInTrackingRange.h"

#include "AI/Controller/SFEnemyController.h"

UBTD_IsInTrackingRange::UBTD_IsInTrackingRange()
{
	NodeName = "Is In Tracking Range";

	// Observer Aborts 기본값
	FlowAbortMode = EBTFlowAbortMode::LowerPriority;

	//Tick 활성화
	bNotifyTick = true;
}

// ========================================
// CalculateRawConditionValue
// ========================================
bool UBTD_IsInTrackingRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	if (ASFEnemyController* AI = Cast<ASFEnemyController>(OwnerComp.GetAIOwner()))
	{
		return AI->IsInTrackingRange();
	}
	return false;
}

// ========================================
// TickNode (실시간 재평가)
// ========================================
void UBTD_IsInTrackingRange::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	FBTTrackingRangeMemory* Memory = reinterpret_cast<FBTTrackingRangeMemory*>(NodeMemory);
	if (!Memory)
	{
		return;
	}

	// 현재 조건 값 계산
	const bool bCurrentResult = CalculateRawConditionValue(OwnerComp, NodeMemory);

	// 값이 바뀌었을 때만 재평가 요청!
	if (Memory->bLastResult != bCurrentResult)
	{
		Memory->bLastResult = bCurrentResult;
		OwnerComp.RequestExecution(this);
	}
}
