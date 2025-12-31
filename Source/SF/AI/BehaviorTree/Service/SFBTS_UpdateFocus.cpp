// SFBTS_UpdateFocus.cpp
#include "SFBTS_UpdateFocus.h"

#include "AbilitySystemComponent.h"
#include "AI/Controller/SFBaseAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AbilitySystemGlobals.h"
#include "Character/SFCharacterGameplayTags.h"
#include "Character/Enemy/SFEnemyGameplayTags.h"

USFBTS_UpdateFocus::USFBTS_UpdateFocus()
{
    NodeName = "Update Focus (Look At Target)";
    bNotifyTick = true;
    bNotifyBecomeRelevant = true; // 서비스 시작 시점 감지 추가
    bNotifyCeaseRelevant = true;
    
    Interval = 0.1f; 
    RandomDeviation = 0.02f; // 약간의 무작위성을 주어 성능 분산

    BlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(USFBTS_UpdateFocus, BlackboardKey), AActor::StaticClass());
}

void USFBTS_UpdateFocus::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::OnBecomeRelevant(OwnerComp, NodeMemory);
    
    // 서비스가 시작될 때 즉시 한번 체크
    UpdateFocusTarget(OwnerComp);
}

void USFBTS_UpdateFocus::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
    UpdateFocusTarget(OwnerComp);
}

void USFBTS_UpdateFocus::UpdateFocusTarget(UBehaviorTreeComponent& OwnerComp)
{
    ASFBaseAIController* AIC = Cast<ASFBaseAIController>(OwnerComp.GetAIOwner());
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();

    if (!AIC || !Blackboard) return;

    // Ability 사용 중에는 Focus 업데이트 중단
    APawn* MyPawn = AIC->GetPawn();
    if (MyPawn)
    {
        if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(MyPawn))
        {
            if (ASC->HasMatchingGameplayTag(SFGameplayTags::Character_State_UsingAbility))
            {
                return;
            }
        }
    }

    AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(BlackboardKey.SelectedKeyName));
    AActor* CurrentFocus = AIC->GetFocusActor();

    if (TargetActor)
    {
        if (CurrentFocus != TargetActor)
        {
           
            AIC->SetFocus(TargetActor, EAIFocusPriority::Gameplay);
        }
    }
    else
    {
        if (CurrentFocus)
        {
            AIC->ClearFocus(EAIFocusPriority::Gameplay);
            AIC->SetRotationMode(EAIRotationMode::MovementDirection);
        }
    }
}

void USFBTS_UpdateFocus::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    // 서비스 범위를 벗어날 때(노드 종료 시) 포커스 해제
    if (AAIController* AIC = OwnerComp.GetAIOwner())
    {
       AIC->ClearFocus(EAIFocusPriority::Gameplay);
    }
    
    Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}