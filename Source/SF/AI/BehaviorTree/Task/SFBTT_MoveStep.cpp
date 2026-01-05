#include "SFBTT_MoveStep.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "AbilitySystem/Abilities/SFGameplayAbilityTags.h"
#include "AbilitySystem/Abilities/Enemy/State/SFGA_MoveStep.h"
#include "AbilitySystem/GameplayEvent/SFGameplayEventTags.h"
#include "AI/SFAIGameplayTags.h"
#include "AI/Controller/Dragon/SFDragonCombatComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/SFCharacterGameplayTags.h"

USFBTT_MoveStep::USFBTT_MoveStep()
{
    NodeName = "Move Step Task";

    TargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(USFBTT_MoveStep, TargetKey), AActor::StaticClass());
    DistanceKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(USFBTT_MoveStep, DistanceKey));
    MinRangeKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(USFBTT_MoveStep, MinRangeKey));
    MaxRangeKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(USFBTT_MoveStep, MaxRangeKey));
    ZoneKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(USFBTT_MoveStep, ZoneKey), StaticEnum<EBossAttackZone>());
}

EBTNodeResult::Type USFBTT_MoveStep::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon) return EBTNodeResult::Failed;

    APawn* Pawn = AICon->GetPawn();
    if (!Pawn) return EBTNodeResult::Failed;

    UAbilitySystemComponent* ASC = Pawn->FindComponentByClass<UAbilitySystemComponent>();
    if (!ASC) return EBTNodeResult::Failed;

    AActor* Target = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TargetKey.SelectedKeyName));
    if (!Target) return EBTNodeResult::Failed;
    
    float Distance = OwnerComp.GetBlackboardComponent()->GetValueAsFloat(DistanceKey.SelectedKeyName);
    if (Distance <= 0.f)
    {
        Distance = Target->GetDistanceTo(Pawn);
    }
    float MinRange = OwnerComp.GetBlackboardComponent()->GetValueAsFloat(MinRangeKey.SelectedKeyName);
    float MaxRange = OwnerComp.GetBlackboardComponent()->GetValueAsFloat(MaxRangeKey.SelectedKeyName);  

    EBossAttackZone Zone = static_cast<EBossAttackZone>(OwnerComp.GetBlackboardComponent()->GetValueAsEnum(ZoneKey.SelectedKeyName));
    if (Zone >= EBossAttackZone::Long)
    {
        return EBTNodeResult::Failed;
    }
    
    float StepDirection = 0.f;
    bool bShouldUseStep = false;
    
    if (Distance < MinRange) // 너무 가까우면 백스탭 (-1)
    {
        StepDirection = -1.f;
        bShouldUseStep = true;
    }
    else if (Distance > MaxRange) // 너무 멀면 전진 스텝 (+1)
    {
        StepDirection = 1.f;
        bShouldUseStep = true;
    }
    
    if (!bShouldUseStep)
    {
        return EBTNodeResult::Failed; 
    }
    
    // [중요] 어빌리티 종료 델리게이트 먼저 등록
    AbilityEndedHandle = ASC->OnAbilityEnded.AddUObject(this, &USFBTT_MoveStep::OnAbilityEnded, TWeakObjectPtr<UBehaviorTreeComponent>(&OwnerComp));
    
    FGameplayEventData Payload;
    Payload.EventTag = SFGameplayTags::GameplayEvent_MoveStep;
    Payload.EventMagnitude = StepDirection;
    
    // 이벤트 전송
    int32 TriggeredAbilities = ASC->HandleGameplayEvent(Payload.EventTag, &Payload);
    
    // [수정] 실행된 어빌리티가 없으면 즉시 실패 (핸들 찾는 루프 삭제함)
    if (TriggeredAbilities == 0)
    {
        Cleanup(&OwnerComp);
        return EBTNodeResult::Failed;
    }
    
    // 핸들을 못 찾아서 무한 루프 도는 것을 방지하기 위해 
    // TriggeredAbilities > 0 이면 무조건 InProgress 리턴하고 OnAbilityEnded를 믿음
    return EBTNodeResult::InProgress;
}

void USFBTT_MoveStep::OnAbilityEnded(const FAbilityEndedData& EndedData, TWeakObjectPtr<UBehaviorTreeComponent> OwnerCompPtr)
{
    UBehaviorTreeComponent* OwnerComp = OwnerCompPtr.Get();
    if (!OwnerComp) return;

    // [수정] 핸들 비교 대신, 종료된 어빌리티가 MoveStep 태그를 가졌는지 확인
    // 이렇게 하면 RunningAbilityHandle을 못 찾았어도 정상적으로 태스크를 종료할 수 있음
    bool bIsTargetAbility = false;

    if (EndedData.AbilityThatEnded)
    {
        // MoveStep 어빌리티 태그 확인
        if (EndedData.AbilityThatEnded->AbilityTags.HasTag(SFGameplayTags::Ability_Enemy_Movement_Step))
        {
            bIsTargetAbility = true;
        }
    }

    // 혹은 간단하게 어떤 어빌리티든 끝나면 종료시킬 수도 있음 (MoveStep 전용 Task이므로)
    if (bIsTargetAbility)
    {
        Cleanup(OwnerComp);
        FinishLatentTask(*OwnerComp, EBTNodeResult::Succeeded);
    }
}

void USFBTT_MoveStep::Cleanup(UBehaviorTreeComponent* OwnerComp)
{
    if (OwnerComp)
    {
        if (AAIController* AIC = OwnerComp->GetAIOwner())
        {
            if (APawn* Pawn = AIC->GetPawn())
            {
                if (UAbilitySystemComponent* ASC = Pawn->FindComponentByClass<UAbilitySystemComponent>())
                {
                    ASC->OnAbilityEnded.Remove(AbilityEndedHandle);
                }
            }
        }
    }
    AbilityEndedHandle.Reset();
}