// SF/AI/BehaviorTree/Service/Boss/BTService_BossDecisionMaker.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_BossDecisionMaker.generated.h"

/**
 * [보스 전용 타겟팅 서비스]
 * 역할: 오직 "누굴 때릴지(Target)"와 "거리(Distance)"만 계산해서 블랙보드에 넣어줌.
 * 스킬 선택은 이제 Behavior Tree 구조가 담당함.
 */
UCLASS()
class SF_API UBTService_BossDecisionMaker : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_BossDecisionMaker();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	// === [Output] 타겟 정보 ===
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;        // 타겟 액터

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector HasTargetKey;          // 타겟 유무 (Bool)

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector DistanceToTargetKey;   // 타겟 거리

	// === [삭제됨] 스킬 선택 관련 키 (SelectedAbilityTagKey) 삭제 ===

	// === [Config] 설정 값 ===
	UPROPERTY(EditAnywhere, Category = "Config|Target")
	float BossChaseDistance = 999999.0f; 

	UPROPERTY(EditAnywhere, Category = "Config|Target")
	float ScoreDifferenceThreshold = 50.f;

private:
	float CalculateTargetScore(APawn* MyPawn, AActor* Target) const;
};