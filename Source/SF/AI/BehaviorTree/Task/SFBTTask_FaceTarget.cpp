// SFBTTask_FaceTarget.cpp
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

    AcceptableAngle = 10.0f;
}

EBTNodeResult::Type USFBTTask_FaceTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    ASFBaseAIController* AI = Cast<ASFBaseAIController>(OwnerComp.GetAIOwner());
    if (!AI) return EBTNodeResult::Failed;

    AActor* Target = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TargetKey.SelectedKeyName));
    if (!Target) return EBTNodeResult::Failed;

    APawn* Pawn = AI->GetPawn();
    if (!Pawn) return EBTNodeResult::Failed;


    if (ASFDragonController* DragonAI = Cast<ASFDragonController>(AI))
    {
        if (USFTurnInPlaceComponent* TurnComp = DragonAI->GetTurnInPlaceComponent())
        {
          
            AI->ClearFocus(EAIFocusPriority::Gameplay);

            float AngleDiff = FMath::Abs(TurnComp->GetAngleToTarget());

     
            if (AngleDiff <= AcceptableAngle)
            {
                TurnComp->SyncControlRotationToTarget();
                AI->SetRotationMode(EAIRotationMode::ControllerYaw);
                return EBTNodeResult::Succeeded;
            }

           
            TurnComp->RequestTurnToTarget(Target);
            return EBTNodeResult::InProgress;
        }
    }

    AI->SetFocus(Target, EAIFocusPriority::Gameplay);
    return EBTNodeResult::InProgress;
}

void USFBTTask_FaceTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    ASFBaseAIController* AI = Cast<ASFBaseAIController>(OwnerComp.GetAIOwner());
    if (!AI) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

    AActor* Target = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TargetKey.SelectedKeyName));
    if (!Target) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }


    if (ASFDragonController* DragonAI = Cast<ASFDragonController>(AI))
    {
        if (USFTurnInPlaceComponent* TurnComp = DragonAI->GetTurnInPlaceComponent())
        {
         
            if (TurnComp->IsTurning()) return;

            float AngleDiff = FMath::Abs(TurnComp->GetAngleToTarget());

 
            if (AngleDiff <= AcceptableAngle)
            {
                TurnComp->SyncControlRotationToTarget();
                AI->SetRotationMode(EAIRotationMode::ControllerYaw);
                FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
                return;
            }

 
            if (AngleDiff < TurnComp->GetTurnThreshold())
            {
                TurnComp->SyncControlRotationToTarget();
            }

            return;
        }
    }


    APawn* Pawn = AI->GetPawn();
    if (!Pawn) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

    FRotator ControllerRot = AI->GetControlRotation();
    FRotator ActorRot = Pawn->GetActorRotation();
    float AngleDiff = FMath::Abs(FMath::FindDeltaAngleDegrees(ActorRot.Yaw, ControllerRot.Yaw));
    if (AngleDiff <= AcceptableAngle)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}

void USFBTTask_FaceTarget::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    ASFBaseAIController* AI = Cast<ASFBaseAIController>(OwnerComp.GetAIOwner());
    if (AI)
    {
        AI->ClearFocus(EAIFocusPriority::Gameplay);
    }

    Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}