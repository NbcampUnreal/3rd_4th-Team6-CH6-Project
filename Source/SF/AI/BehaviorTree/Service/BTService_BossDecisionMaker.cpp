// SF/AI/BehaviorTree/Service/Boss/BTService_BossDecisionMaker.cpp

#include "BTService_BossDecisionMaker.h" 

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
// #include "AI/Controller/SFEnemyCombatComponent.h" // 더 이상 필요 없음
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"

UBTService_BossDecisionMaker::UBTService_BossDecisionMaker()
{
	NodeName = "Boss Target Tracker"; // 이름 변경 추천
	
	Interval = 0.1f; 
	RandomDeviation = 0.0f; 

	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_BossDecisionMaker, TargetActorKey), AActor::StaticClass());
	HasTargetKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_BossDecisionMaker, HasTargetKey));
	DistanceToTargetKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_BossDecisionMaker, DistanceToTargetKey));
    // AbilityTagKey 필터 삭제
}

float UBTService_BossDecisionMaker::CalculateTargetScore(APawn* MyPawn, AActor* Target) const
{
	if (!MyPawn || !Target) return -1.f;
	float Distance = FVector::Dist(MyPawn->GetActorLocation(), Target->GetActorLocation());
	return FMath::Clamp(2000.f - (Distance / 5.f), 0.f, 2000.f);
}

void UBTService_BossDecisionMaker::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!Blackboard || !AIC) return;

	APawn* MyPawn = AIC->GetPawn();
	if (!MyPawn) return;

	// =========================================================
	// [Part 1] 타겟 관리 (Target Selection) - 유지!
	// =========================================================
	AActor* CurrentTarget = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
	
	if (CurrentTarget)
	{
		float Dist = FVector::Dist(MyPawn->GetActorLocation(), CurrentTarget->GetActorLocation());
		if (Dist > BossChaseDistance || !IsValid(CurrentTarget))
		{
			CurrentTarget = nullptr; 
		}
	}

	TArray<AActor*> PerceivedActors;
	if (auto* PerceptionComp = AIC->GetPerceptionComponent())
	{
		PerceptionComp->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActors);
	}

	AActor* BestTarget = nullptr;
	float BestScore = -1.f;

	for (AActor* Actor : PerceivedActors)
	{
		if (Actor && Actor->ActorHasTag("Player")) 
		{
			float Score = CalculateTargetScore(MyPawn, Actor);
			if (Score > BestScore)
			{
				BestScore = Score;
				BestTarget = Actor;
			}
		}
	}

	if (BestTarget)
	{
		if (!CurrentTarget || (CurrentTarget && BestTarget != CurrentTarget && (BestScore - CalculateTargetScore(MyPawn, CurrentTarget)) >= ScoreDifferenceThreshold))
		{
			CurrentTarget = BestTarget;
		}
	}

	// 결과 저장
	if (CurrentTarget)
	{
		Blackboard->SetValueAsObject(TargetActorKey.SelectedKeyName, CurrentTarget);
		Blackboard->SetValueAsBool(HasTargetKey.SelectedKeyName, true);
		float DistToTarget = FVector::Dist(MyPawn->GetActorLocation(), CurrentTarget->GetActorLocation());
		Blackboard->SetValueAsFloat(DistanceToTargetKey.SelectedKeyName, DistToTarget);
		AIC->SetFocus(CurrentTarget);
	}
	else
	{
		Blackboard->ClearValue(TargetActorKey.SelectedKeyName);
		Blackboard->SetValueAsBool(HasTargetKey.SelectedKeyName, false);
		// 거리값도 초기화해주면 좋음 (선택 사항)
		// Blackboard->ClearValue(DistanceToTargetKey.SelectedKeyName); 
		AIC->ClearFocus(EAIFocusPriority::Gameplay);
	}

	// =========================================================
	// [Part 2] 스킬 선택 로직 -> 완전히 삭제됨!
	// 이제 비헤이비어 트리의 Selector와 Cooldown Decorator가 결정합니다.
	// =========================================================
}