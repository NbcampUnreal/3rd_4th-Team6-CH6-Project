#include "AI/BehaviorTree/Task/SFBTTask_FindAttackPoint.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "NavigationSystem.h"
#include "Character/SFCharacterBase.h"
#include "AbilitySystem/SFAbilitySystemComponent.h"
#include "AbilitySystem/GameplayEvent/SFGameplayEventTags.h"

USFBTTask_FindAttackPoint::USFBTTask_FindAttackPoint(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    NodeName = "Find Attack Point (EQS)";
    bCreateNodeInstance = true;
    ResultKeyName.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(USFBTTask_FindAttackPoint, ResultKeyName));
    TargetActor.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(USFBTTask_FindAttackPoint, TargetActor), AActor::StaticClass());
    AbilityTagKeyName.AddNameFilter(this, GET_MEMBER_NAME_CHECKED(USFBTTask_FindAttackPoint, AbilityTagKeyName));
}

EBTNodeResult::Type USFBTTask_FindAttackPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    ASFCharacterBase* Character = AIController ? Cast<ASFCharacterBase>(AIController->GetPawn()) : nullptr;
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();

    if (!Character || !Blackboard || !QueryTemplate) return EBTNodeResult::Failed;

    AActor* TargetActorPtr = Cast<AActor>(Blackboard->GetValueAsObject(TargetActor.SelectedKeyName));
    FName TagName = Blackboard->GetValueAsName(AbilityTagKeyName.SelectedKeyName);
    FGameplayTag AbilityTag = FGameplayTag::RequestGameplayTag(TagName);

    if (!TargetActorPtr || !AbilityTag.IsValid()) return EBTNodeResult::Failed;
    
    float MinDist = 0.f;
    float MaxDist = 1000.f;
    USFAbilitySystemComponent* ASC = Character->GetSFAbilitySystemComponent();
    
    if (ASC)
    {
        for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
        {
            if (Spec.Ability && Spec.Ability->AbilityTags.HasTag(AbilityTag))
            {
                const float* MinValPtr = Spec.SetByCallerTagMagnitudes.Find(SFGameplayTags::Data_EnemyAbility_MinAttackRange);
                const float* MaxValPtr = Spec.SetByCallerTagMagnitudes.Find(SFGameplayTags::Data_EnemyAbility_AttackRange);

                if (MinValPtr && MaxValPtr)
                {
                    MinDist = *MinValPtr;
                    MaxDist = *MaxValPtr;
                }
                break;
            }
        }
    }
    
    if (bSkipIfInRange)
    {
        float DistSq = FVector::DistSquared2D(Character->GetActorLocation(), TargetActorPtr->GetActorLocation());
        if (DistSq >= (MinDist * MinDist) && DistSq <= (MaxDist * MaxDist * 0.81f))
        {
            return EBTNodeResult::Succeeded;
        }
    }
    
    CachedOwnerComp = &OwnerComp;
    FEnvQueryRequest QueryRequest(QueryTemplate, Character);
    
    QueryRequest.SetFloatParam(FName("MinDistance"), MinDist);
    QueryRequest.SetFloatParam(FName("MaxDistance"), MaxDist);
    QueryRequest.SetFloatParam(FName("OptimalDistance"), (MinDist + MaxDist) * 0.5f);

    QueryID = QueryRequest.Execute(RunMode, this, &USFBTTask_FindAttackPoint::OnQueryFinished);

    return (QueryID == INDEX_NONE) ? EBTNodeResult::Failed : EBTNodeResult::InProgress;
}

void USFBTTask_FindAttackPoint::OnQueryFinished(TSharedPtr<FEnvQueryResult> Result)
{
    if (!CachedOwnerComp.IsValid() || QueryID == INDEX_NONE || Result->QueryID != QueryID) return;

    QueryID = INDEX_NONE;

    if (Result->IsAborted())
    {
        FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Aborted);
        return;
    }

    if (Result->IsSuccessful())
    {
        FVector ResultLocation = Result->GetItemAsLocation(0);

        // 4. 드래곤을 위한 정교한 NavMesh 투영
        if (bProjectToNavMesh)
        {
            if (UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
            {
                FNavLocation ProjectedLocation;
                // 드래곤의 크기를 고려하여 투영 범위 설정 
                FVector ProjectionExtent(NavMeshProjectionRadius, NavMeshProjectionRadius, NavMeshProjectionRadius);
                
                if (NavSys->ProjectPointToNavigation(ResultLocation, ProjectedLocation, ProjectionExtent))
                {
                    ResultLocation = ProjectedLocation.Location;
                }
            }
        }

        if (UBlackboardComponent* Blackboard = CachedOwnerComp->GetBlackboardComponent())
        {
            Blackboard->SetValueAsVector(ResultKeyName.SelectedKeyName, ResultLocation);
        }
        FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
    }
    else
    {
        FinishLatentTask(*CachedOwnerComp, FailureResult);
    }
}

EBTNodeResult::Type USFBTTask_FindAttackPoint::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (QueryID != INDEX_NONE)
    {
        if (UEnvQueryManager* QueryManager = UEnvQueryManager::GetCurrent(GetWorld()))
        {
            QueryManager->AbortQuery(QueryID);
        }
        QueryID = INDEX_NONE;
    }
    return Super::AbortTask(OwnerComp, NodeMemory);
}

void USFBTTask_FindAttackPoint::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    QueryID = INDEX_NONE;
    CachedOwnerComp.Reset();
    Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}