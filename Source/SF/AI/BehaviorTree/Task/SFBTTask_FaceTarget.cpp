#include "SFBTTask_FaceTarget.h"
#include "AI/Controller/SFBaseAIController.h"
#include "AI/Controller/Dragon/SFDragonController.h"
#include "AI/Controller/SFTurnInPlaceComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"

USFBTTask_FaceTarget::USFBTTask_FaceTarget()
{
    NodeName = "SF Face Target";
    TargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(USFBTTask_FaceTarget, TargetKey), AActor::StaticClass());
    bNotifyTick = true;
    bNotifyTaskFinished = true;
    AcceptableAngle = 5.0f;
}

EBTNodeResult::Type USFBTTask_FaceTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    ASFBaseAIController* AI = Cast<ASFBaseAIController>(OwnerComp.GetAIOwner());
    if (!AI) return EBTNodeResult::Failed;

    AActor* Target = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TargetKey.SelectedKeyName));
    if (!Target) return EBTNodeResult::Failed;

    if (ASFDragonController* DragonAI = Cast<ASFDragonController>(AI))
    {
        if (USFTurnInPlaceComponent* TurnComp = DragonAI->GetTurnInPlaceComponent())
        {
            // 실행하자마자 각도 체크
            float AngleDiff = FMath::Abs(TurnComp->GetAngleToTarget());

            // 이미 각도 안에 있고, 애니메이션 중도 아니라면 즉시 성공
            if (AngleDiff <= AcceptableAngle && !TurnComp->IsTurning())
            {
                return EBTNodeResult::Succeeded;
            }

            // 그 외에는 무조건 InProgress로 넘겨서 Tick을 타게 함
            TurnComp->RequestTurnToTarget(Target);
            return EBTNodeResult::InProgress;
        }
    }

    return EBTNodeResult::InProgress;
}

void USFBTTask_FaceTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    ASFBaseAIController* AI = Cast<ASFBaseAIController>(OwnerComp.GetAIOwner());
    AActor* Target = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TargetKey.SelectedKeyName));
    if (!AI || !Target) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

    if (ASFDragonController* DragonAI = Cast<ASFDragonController>(AI))
    {
        if (USFTurnInPlaceComponent* TurnComp = DragonAI->GetTurnInPlaceComponent())
        {
            // 1. [매우 중요] 애니메이션(TIP) 중이면 각도와 상관없이 태스크를 끝내지 않음
            if (TurnComp->IsTurning()) 
            {
                return; 
            }

            // 2. 현재 각도 체크
            float AngleDiff = FMath::Abs(TurnComp->GetAngleToTarget());
            
            // 3. 각도 만족 시 종료
            if (AngleDiff <= AcceptableAngle)
            {
                FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
                return;
            }

            // 4. 아직 각도가 안 맞으면 계속 회전 요청
            TurnComp->RequestTurnToTarget(Target);
            return; 
        }
    }
    
    // 일반 AI 로직 (Focus 사용 시)
    APawn* Pawn = AI->GetPawn();
    if (Pawn)
    {
        float AngleDiff = FMath::Abs(FMath::FindDeltaAngleDegrees(Pawn->GetActorRotation().Yaw, AI->GetControlRotation().Yaw));
        if (AngleDiff <= AcceptableAngle)
        {
            FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        }
    }
}

void USFBTTask_FaceTarget::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}