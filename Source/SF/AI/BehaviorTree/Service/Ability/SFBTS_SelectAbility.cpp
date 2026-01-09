// AI/BehaviorTree/Service/Ability/SFBTS_SelectAbility.cpp

#include "SFBTS_SelectAbility.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h" // [필수 추가] ASC 가져오기 위해 필요
#include "AIController.h"
#include "AI/Controller/SFEnemyCombatComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Interface/SFEnemyAbilityInterface.h"
#include "Character/SFCharacterGameplayTags.h" // [필수 추가] Attacking 태그 확인용
#include "Interface/SFAIControllerInterface.h"
#include "Navigation/PathFollowingComponent.h" // [추가] 이동 상태 체크용

USFBTS_SelectAbility::USFBTS_SelectAbility()
{
    NodeName = TEXT("Select Ability");
    Interval = 0.5f;  
}

uint16 USFBTS_SelectAbility::GetInstanceMemorySize() const
{
    return sizeof(FBTSelectAbilityMemory);
}

void USFBTS_SelectAbility::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const
{
    Super::InitializeMemory(OwnerComp, NodeMemory, InitType);
    
    if (InitType == EBTMemoryInit::Initialize)
    {
        new (NodeMemory) FBTSelectAbilityMemory();
    }
}

void USFBTS_SelectAbility::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) return;

    // [최적화] 이미 선택된 Ability가 있으면 즉시 종료 (Early Return)
    const FName CurrentAbilityTagName = BB->GetValueAsName(BlackboardKey.SelectedKeyName);
    if (!CurrentAbilityTagName.IsNone())
    {
        return;
    }

    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController) return;

    APawn* Pawn = AIController->GetPawn();
    if (!Pawn) return;

    UAbilitySystemComponent* ASC =
        UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Pawn);

    USFCombatComponentBase* CombatComp = nullptr;
    if (ISFAIControllerInterface* AI = Cast<ISFAIControllerInterface>(AIController))
    {
        CombatComp = AI->GetCombatComponent();
    }

    if (!CombatComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SelectAbility] CombatComponent is null"));
        return;
    }


    FEnemyAbilitySelectContext Context;
    Context.Self = Pawn;

    Context.Target =
        Cast<AActor>(BB->GetValueAsObject(TargetKey.SelectedKeyName));

    if (!Context.Target)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SelectAbility] Target is null"));
        return;
    }

    FGameplayTag SelectedTag;

    if (CombatComp->SelectAbility(Context, AbilitySearchTags, SelectedTag))
    {
        UE_LOG(LogTemp, Warning, TEXT("[SelectAbility] SUCCESS: Selected %s"), *SelectedTag.ToString());
        BB->SetValueAsName(
            BlackboardKey.SelectedKeyName,
            SelectedTag.GetTagName()
        );
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[SelectAbility] FAILED: SelectAbility returned false"));
    }
}
