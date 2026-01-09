// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BehaviorTree/Blackboard/BlackboardKeyEnums.h"
#include "SFBTD_CompareDistanceWithAbilityRange.generated.h"

/**
 * 데코레이터 내부에 상태를 저장하기 위한 메모리 구조체
 */
USTRUCT()
struct FBTDistanceCompareMemory
{
	GENERATED_BODY()

	bool bLastResult = false;
};

/**
 * BTDecorator: Blackboard의 Target과의 거리와 특정 Ability의 Range를 비교합니다.
 */
UCLASS()
class SF_API USFBTD_CompareDistanceWithAbilityRange : public UBTDecorator
{
	GENERATED_BODY()

public:
	USFBTD_CompareDistanceWithAbilityRange();

	// 비교 대상이 되는 타겟 (Object Key)
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetKey;

	// 이미 계산된 거리가 있다면 사용 (Float Key, Optional)
	UPROPERTY(EditAnywhere, Category = "Blackboard",meta=(AllowNone = "true"))
	FBlackboardKeySelector DistanceKey;

	// 비교할 어빌리티를 식별할 태그 (Name/Tag Key)
	UPROPERTY(EditAnywhere, Category = "Ability")
	FBlackboardKeySelector AbilityTagKey;


protected:
	virtual uint16 GetInstanceMemorySize() const override;
	virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	// 조건 체크 공통 로직
	bool CheckCondition(UBehaviorTreeComponent& OwnerComp) const;
	void GetAbilityAttackRange(UBehaviorTreeComponent& OwnerComp, const FGameplayTag& AbilityTag, float& OutMinRange,
	                           float& OutMaxRange) const;

    
	// Pawn과 Target 사이의 실제 거리를 계산하는 함수
	float CalculateDistance(UBehaviorTreeComponent& OwnerComp) const;
};