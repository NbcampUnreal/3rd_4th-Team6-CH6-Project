// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Decorators/BTDecorator_BlueprintBase.h"
#include "BTD_IsInMeleeRange.generated.h"

struct FBTMeleeRangeMemory
{
	bool bLastResult = false;
};
/**
 * 
 */
UCLASS()
class SF_API UBTD_IsInMeleeRange : public UBTDecorator_BlueprintBase
{
	GENERATED_BODY()

public:
	UBTD_IsInMeleeRange();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMeleeRangeMemory); }
	
};
