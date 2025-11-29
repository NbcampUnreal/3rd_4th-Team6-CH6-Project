// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BehaviorTree/Decorators/BTD_IsInMeleeRange.h"

#include "AI/Controller/SFEnemyController.h"

UBTD_IsInMeleeRange::UBTD_IsInMeleeRange()
{
	NodeName = "Is In Melee Range";

	// Observer Aborts 기본값
	FlowAbortMode = EBTFlowAbortMode::LowerPriority;

	//Tick 활성화
	bNotifyTick = true;
}

bool UBTD_IsInMeleeRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	if (ASFEnemyController* AI = Cast<ASFEnemyController>(OwnerComp.GetAIOwner()))
	{
		return AI->IsInMeleeRange();
	}
	return false;
}


void UBTD_IsInMeleeRange::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	FBTMeleeRangeMemory* Memory = reinterpret_cast<FBTMeleeRangeMemory*>(NodeMemory);
	if (!Memory)
	{
		return;
	}

	// 현재 조건 값 계산
	const bool bCurrentResult = CalculateRawConditionValue(OwnerComp, NodeMemory);

	//값이 바뀌었을 때만 재평가
	if (Memory->bLastResult != bCurrentResult)
	{
		Memory->bLastResult = bCurrentResult;
		OwnerComp.RequestExecution(this);
	}
}