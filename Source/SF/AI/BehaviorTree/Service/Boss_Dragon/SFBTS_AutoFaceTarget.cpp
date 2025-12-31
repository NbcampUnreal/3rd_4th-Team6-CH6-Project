#include "SFBTS_AutoFaceTarget.h"

#include "AI/Controller/SFBaseAIController.h"
#include "AI/Controller/Dragon/SFDragonController.h"
#include "AI/Controller/SFTurnInPlaceComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Character/SFCharacterGameplayTags.h"
#include "GameFramework/Pawn.h"
#include "Navigation/PathFollowingComponent.h"

USFBTS_AutoFaceTarget::USFBTS_AutoFaceTarget()
{
    NodeName = "Auto Face Target ";

    Interval = 0.1f;
    RandomDeviation = 0.0f;

    TargetKey.AddObjectFilter(
        this,
        GET_MEMBER_NAME_CHECKED(USFBTS_AutoFaceTarget, TargetKey),
        AActor::StaticClass()
    );

    AcceptableAngle = 10.f;        
    MovementSpeedThreshold = 5.f;   
    bSkipDuringAbility = true;
}

void USFBTS_AutoFaceTarget::TickNode(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory,
    float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    ASFBaseAIController* AI = Cast<ASFBaseAIController>(OwnerComp.GetAIOwner());
    if (!AI) return;

    APawn* Pawn = AI->GetPawn();
    if (!Pawn) return;

    AActor* Target =
        Cast<AActor>(OwnerComp.GetBlackboardComponent()
                         ->GetValueAsObject(TargetKey.SelectedKeyName));
    if (!Target) return;


    if (const UPathFollowingComponent* PathComp = AI->GetPathFollowingComponent())
    {
        if (PathComp->GetStatus() == EPathFollowingStatus::Moving)
        {
            return;
        }
    }

  
    if (bSkipDuringAbility)
    {
        if (UAbilitySystemComponent* ASC =
                UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Pawn))
        {
            if (ASC->HasMatchingGameplayTag(SFGameplayTags::Character_State_Attacking) ||
                ASC->HasMatchingGameplayTag(SFGameplayTags::Character_State_UsingAbility) ||
                ASC->HasMatchingGameplayTag(SFGameplayTags::Character_State_TurningInPlace))
            {
                return;
            }
        }
    }


    if (Pawn->GetVelocity().Size2D() > MovementSpeedThreshold)
    {
        return;
    }

   
    if (ASFDragonController* DragonAI = Cast<ASFDragonController>(AI))
    {
        USFTurnInPlaceComponent* TurnComp = DragonAI->GetTurnInPlaceComponent();
        if (!TurnComp) return;

        if (TurnComp->IsTurning()) return;

        const float AngleDiff =
            FMath::Abs(TurnComp->GetAngleToTarget());
        
        if (AngleDiff <= AcceptableAngle)
        {
            return;
        }
        
        if (AngleDiff >= TurnComp->GetTurnThreshold())
        {
            TurnComp->RequestTurnToTarget(Target);
        }
        
        else
        {
            TurnComp->SyncControlRotationToTarget();
        }
    }
    else
    {
        
        AI->SetFocus(Target, EAIFocusPriority::Gameplay);
    }
}
