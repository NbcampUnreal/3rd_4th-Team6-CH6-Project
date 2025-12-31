#include "SFBTS_UpdateTargetData.h"
#include "AI/Controller/Dragon/SFDragonCombatComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "AI/Controller/SFBaseAIController.h"

USFBTS_UpdateTargetData::USFBTS_UpdateTargetData()
{
    NodeName = "Update Target Data ";
    Interval = 0.5f;
    RandomDeviation = 0.1f;

    BlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(USFBTS_UpdateTargetData, BlackboardKey), AActor::StaticClass());

    DistanceKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(USFBTS_UpdateTargetData, DistanceKey));

    AngleKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(USFBTS_UpdateTargetData, AngleKey));

    ZoneKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(USFBTS_UpdateTargetData, ZoneKey),
        StaticEnum<EBossAttackZone>());

    bNotifyBecomeRelevant = true;
    bNotifyCeaseRelevant = true;
    bNotifyTick = true;
}

void USFBTS_UpdateTargetData::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    if (!CombatComponent) return;

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) return;

    AActor* NewTarget = CombatComponent->GetCurrentTarget();

    // 1. 타겟 액터 업데이트 (컴포넌트에서 이미 하지만, 동기화를 위해 확인)
    // 에디터에서 선택한 키 이름이 "TargetActor"와 같은지 반드시 확인하세요!
    BB->SetValueAsObject(GetSelectedBlackboardKey(), NewTarget);

    if (NewTarget)
    {
        // 2. 부가 정보 업데이트
        BB->SetValueAsFloat(DistanceKey.SelectedKeyName, CombatComponent->GetDistanceToTarget());
        BB->SetValueAsFloat(AngleKey.SelectedKeyName, CombatComponent->GetAngleToTarget());
        BB->SetValueAsEnum(ZoneKey.SelectedKeyName, static_cast<uint8>(CombatComponent->GetTargetLocationZone()));
    }
    else
    {
        // 3. 타겟이 없으면 부가 정보 초기화 (이걸 안 하면 멀리 도망가도 예전 거리값이 남음)
        BB->SetValueAsFloat(DistanceKey.SelectedKeyName, 0.f);
        BB->SetValueAsFloat(AngleKey.SelectedKeyName, 0.f);
        BB->SetValueAsEnum(ZoneKey.SelectedKeyName, static_cast<uint8>(EBossAttackZone::None));
    }
}

void USFBTS_UpdateTargetData::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::OnBecomeRelevant(OwnerComp, NodeMemory);
    
    AAIController* AIC = OwnerComp.GetAIOwner();
    if (AIC)
    {
        CombatComponent = AIC->FindComponentByClass<USFDragonCombatComponent>();
    }
}

void USFBTS_UpdateTargetData::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::OnCeaseRelevant(OwnerComp, NodeMemory);
    
}