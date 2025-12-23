// SF/AI/BehaviorTree/Service/BTService_UpdateTarget.cpp

#include "BTService_UpdateTarget.h"

#include "AIController.h"
#include "AI/Controller/SFEnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "GameFramework/Pawn.h"

UBTService_UpdateTarget::UBTService_UpdateTarget()
{
	NodeName = "Update Target";
	Interval = 0.2f; 
	RandomDeviation = 0.05f;

	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateTarget, TargetActorKey), AActor::StaticClass());
	HasTargetKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateTarget, HasTargetKey));
}

float UBTService_UpdateTarget::CalculateTargetScore(UBehaviorTreeComponent& OwnerComp, AActor* Target, ASFEnemyController* AIController) const
{
	if (!Target || !AIController) return -1.f;
	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn) return -1.f;

	const float Distance = FVector::Dist(ControlledPawn->GetActorLocation(), Target->GetActorLocation());
	// 가까울수록 높은 점수
	return FMath::Clamp(1000.f - (Distance / 10.f), 0.f, 1000.f);
}

void UBTService_UpdateTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	ASFEnemyController* AIController = Cast<ASFEnemyController>(OwnerComp.GetAIOwner());
	if (!AIController) return;

	APawn* MyPawn = AIController->GetPawn();
	if (!MyPawn) return;

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp) return;

	AActor* CurrentTarget = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));

	// 1. 현재 타겟과의 거리 체크
	// (MaxChaseDistance를 999999로 늘렸으므로, 이제 여기서 타겟이 갑자기 사라지지 않습니다)
	if (CurrentTarget)
	{
		float Dist = FVector::Dist(MyPawn->GetActorLocation(), CurrentTarget->GetActorLocation());
		
		if (Dist > MaxChaseDistance)
		{
			// 너무 멀어졌을 때만 타겟 해제
			BlackboardComp->ClearValue(TargetActorKey.SelectedKeyName);
			BlackboardComp->SetValueAsBool(HasTargetKey.SelectedKeyName, false);
			AIController->TargetActor = nullptr;
			return; 
		}
	}

	// 2. 시야(Perception)에 보이는 적들 탐색
	TArray<AActor*> PerceivedActors;
	if (auto* PerceptionComp = AIController->GetPerceptionComponent())
	{
		PerceptionComp->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActors);
	}

	AActor* BestTarget = nullptr;
	float BestScore = -1.f;

	if (PerceivedActors.Num() > 0)
	{
		for (AActor* Actor : PerceivedActors)
		{
			// 플레이어 태그 확인
			if (!Actor->ActorHasTag(FName("Player"))) continue;

			const float ActorScore = CalculateTargetScore(OwnerComp, Actor, AIController);
			if (ActorScore > BestScore)
			{
				BestScore = ActorScore;
				BestTarget = Actor;
			}
		}
	}

	// 3. 타겟 갱신 로직 (더 좋은 타겟이 있거나, 타겟이 없을 때)
	if (BestTarget)
	{
		bool bShouldSwitch = false;

		if (CurrentTarget && BestTarget != CurrentTarget)
		{
			float CurrentScore = CalculateTargetScore(OwnerComp, CurrentTarget, AIController);
			// 점수 차이가 날 때만 교체
			if ((BestScore - CurrentScore) >= ScoreDifferenceThreshold)
			{
				bShouldSwitch = true;
			}
		}
		else if (!CurrentTarget)
		{
			bShouldSwitch = true;
		}

		if (bShouldSwitch)
		{
			CurrentTarget = BestTarget;
			
			BlackboardComp->SetValueAsObject(TargetActorKey.SelectedKeyName, CurrentTarget);
			BlackboardComp->SetValueAsBool(HasTargetKey.SelectedKeyName, true);
			AIController->TargetActor = CurrentTarget;
			BlackboardComp->SetValueAsVector("LastKnownPosition", CurrentTarget->GetActorLocation());
		}
	}
	else if (!CurrentTarget)
	{
		// 타겟도 없고, 보이는 적도 없으면 클리어
		BlackboardComp->ClearValue(TargetActorKey.SelectedKeyName);
		BlackboardComp->SetValueAsBool(HasTargetKey.SelectedKeyName, false);
		AIController->TargetActor = nullptr;
		return;
	}

	// 타겟 추적 중이면 마지막 위치 업데이트
	if (CurrentTarget)
	{
		BlackboardComp->SetValueAsVector("LastKnownPosition", CurrentTarget->GetActorLocation());
	}
}