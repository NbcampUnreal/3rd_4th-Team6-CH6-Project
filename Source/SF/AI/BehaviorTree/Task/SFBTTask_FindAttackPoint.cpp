#include "SFBTTask_FindAttackPoint.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "NavigationSystem.h"
#include "VisualLogger/VisualLogger.h"

// Game Specific Headers
#include "Character/SFCharacterBase.h"
#include "AbilitySystem/SFAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/Enemy/Combat/SFGA_Enemy_BaseAttack.h"

USFBTTask_FindAttackPoint::USFBTTask_FindAttackPoint(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    NodeName = "Find Attack Point (EQS)";
    
    // 필터 설정 (에디터 편의성)
    ResultKeyName.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(USFBTTask_FindAttackPoint, ResultKeyName));
    TargetActor.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(USFBTTask_FindAttackPoint, TargetActor), AActor::StaticClass());
    AbilityTagKeyName.AddNameFilter(this, GET_MEMBER_NAME_CHECKED(USFBTTask_FindAttackPoint, AbilityTagKeyName));
}

EBTNodeResult::Type USFBTTask_FindAttackPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    ASFCharacterBase* Character = AIController ? Cast<ASFCharacterBase>(AIController->GetPawn()) : nullptr;
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();

    if (!Character || !Blackboard || !QueryTemplate)
    {
        return EBTNodeResult::Failed;
    }

    CachedOwnerComp = &OwnerComp;
    
    AActor* TargetActorPtr = Cast<AActor>(Blackboard->GetValueAsObject(TargetActor.SelectedKeyName));
    FGameplayTag AbilityTag = FGameplayTag::RequestGameplayTag(Blackboard->GetValueAsName(AbilityTagKeyName.SelectedKeyName)); 

    if (!TargetActorPtr || !AbilityTag.IsValid())
    {
        return EBTNodeResult::Failed;
    }

    // 2. ASC에서 해당 태그를 가진 어빌리티(GA)를 찾아 사거리 정보 가져오기
    float MinDist = 0.f;
    float MaxDist = 1000.f;
    bool bFoundAbility = false;

    USFAbilitySystemComponent* ASC = Character->GetSFAbilitySystemComponent();
    if (ASC)
    {
        // ActivatableAbilities를 순회하며 태그가 일치하는 인스턴스 찾기
        for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
        {
            if (Spec.Ability && Spec.Ability->AbilityTags.HasTag(AbilityTag))
            {
                // 인스턴싱된 어빌리티라면 PrimaryInstance를 가져옴
                USFGA_Enemy_BaseAttack* AttackAbility = Cast<USFGA_Enemy_BaseAttack>(Spec.GetPrimaryInstance());
                
                // 만약 인스턴싱되지 않은 어빌리티(CDO 사용)라면 CDO를 캐스팅
                if (!AttackAbility)
                {
                    AttackAbility = Cast<USFGA_Enemy_BaseAttack>(Spec.Ability);
                }

                if (AttackAbility)
                {
                    MinDist = AttackAbility->GetMinAttackRange();
                    MaxDist = AttackAbility->GetAttackRange();
                    bFoundAbility = true;
                    break;
                }
            }
        }
    }

    if (!bFoundAbility)
    {
        UE_VLOG(Character, LogBehaviorTree, Warning, TEXT("SFBTTask_FindAttackPoint: Could not find Ability with Tag %s"), *AbilityTag.ToString());
        // 실패 처리하거나 기본값으로 진행할 수 있습니다. 여기선 기본값(0~1000)으로 진행합니다.
    }

    // 3. (옵션) 이미 사거리 내에 있다면 스킵
    // 주석에 언급하신 대로 몹이 뭉치는 것을 방지하기 위해 이 옵션은 보통 꺼두거나(false),
    // bSkipIfInRange가 true일 때만 체크하도록 합니다.
    if (bSkipIfInRange && TargetActorPtr)
    {
        float DistSq = FVector::DistSquared(Character->GetActorLocation(), TargetActorPtr->GetActorLocation());
        float MaxRangeSq = MaxDist * MaxDist;
        // 약간의 여유(Tolerance)를 두고 체크
        if (DistSq < (MaxRangeSq * 0.9f)) 
        {
            // 이미 공격 가능 범위 안이라면 이동할 필요 없음 -> 성공 반환
            return EBTNodeResult::Succeeded;
        }
    }

    // 4. EQS 쿼리 요청 생성
    FEnvQueryRequest QueryRequest(QueryTemplate, Character);

    // 최적 거리 계산 (Min과 Max의 중간)
    const float OptimalDistance = (MinDist + MaxDist) * 0.5f;

    // EQS 파라미터 주입 (EQS 에셋에서 이 이름의 파라미터를 사용해야 함)
    QueryRequest.SetFloatParam(FName("MinDistance"), MinDist);
    QueryRequest.SetFloatParam(FName("MaxDistance"), MaxDist);
    QueryRequest.SetFloatParam(FName("OptimalDistance"), OptimalDistance);

    // 쿼리 실행
    QueryID = QueryRequest.Execute(RunMode, this, &USFBTTask_FindAttackPoint::OnQueryFinished);

    if (QueryID == INDEX_NONE)
    {
        return EBTNodeResult::Failed;
    }

    return EBTNodeResult::InProgress;
}

void USFBTTask_FindAttackPoint::OnQueryFinished(TSharedPtr<FEnvQueryResult> Result)
{
    if (!CachedOwnerComp.IsValid() || QueryID == INDEX_NONE)
    {
        return;
    }

    // 현재 실행 중인 쿼리 결과인지 확인
    if (Result->QueryID != QueryID)
    {
        return;
    }

    QueryID = INDEX_NONE;
    
    if (Result->IsAborted())
    {
        FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Aborted);
        return;
    }

    if (Result->IsSuccessful())
    {
        FVector ResultLocation = Result->GetItemAsLocation(0);
        UBlackboardComponent* Blackboard = CachedOwnerComp->GetBlackboardComponent();

        // NavMesh 투영 (참고 코드의 로직 반영)
        if (bProjectToNavMesh)
        {
            const UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
            if (NavSys)
            {
                FNavLocation ProjectedLocation;
                if (NavSys->ProjectPointToNavigation(ResultLocation, ProjectedLocation, FVector(200.f, 200.f, 200.f)))
                {
                    ResultLocation = ProjectedLocation.Location;
                }
            }
        }

        // 결과 블랙보드에 쓰기
        if (Blackboard)
        {
            Blackboard->SetValueAsVector(ResultKeyName.SelectedKeyName, ResultLocation);
        }

        FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
    }
    else
    {
        // 쿼리 실패 시 지정된 실패 결과 반환
        FinishLatentTask(*CachedOwnerComp, FailureResult);
    }
}

EBTNodeResult::Type USFBTTask_FindAttackPoint::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (QueryID != INDEX_NONE)
    {
        UEnvQueryManager* QueryManager = UEnvQueryManager::GetCurrent(GetWorld());
        if (QueryManager)
        {
            QueryManager->AbortQuery(QueryID);
        }
        QueryID = INDEX_NONE;
    }

    return Super::AbortTask(OwnerComp, NodeMemory);
}

void USFBTTask_FindAttackPoint::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    CachedOwnerComp.Reset();
    QueryID = INDEX_NONE;

    Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}