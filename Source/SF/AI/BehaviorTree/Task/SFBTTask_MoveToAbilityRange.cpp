// Fill out your copyright notice in the Description page of Project Settings.

#include "SFBTTask_MoveToAbilityRange.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Abilities/Enemy/Combat/SFGA_Enemy_BaseAttack.h"
#include "AbilitySystem/Abilities/SFGameplayAbilityTags.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/GameplayEvent/SFGameplayEventTags.h"

USFBTTask_MoveToAbilityRange::USFBTTask_MoveToAbilityRange()
{
	NodeName = "Move To Ability Range";
	bNotifyTick = true;

	SelectedAbilityTagKey.AddNameFilter(this, GET_MEMBER_NAME_CHECKED(USFBTTask_MoveToAbilityRange, SelectedAbilityTagKey));
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(USFBTTask_MoveToAbilityRange, TargetActorKey), AActor::StaticClass());
	DistanceKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(USFBTTask_MoveToAbilityRange, DistanceKey));
	MoveFailCountKey.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(USFBTTask_MoveToAbilityRange, MoveFailCountKey));
}

EBTNodeResult::Type USFBTTask_MoveToAbilityRange::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	APawn* Pawn = AIController ? AIController->GetPawn() : nullptr;

	if (!AIController || !BB || !Pawn)
		return EBTNodeResult::Failed;

	FName TagName = BB->GetValueAsName(SelectedAbilityTagKey.SelectedKeyName);
	if (TagName == NAME_None)
		return EBTNodeResult::Failed;

	FGameplayTag AbilityTag = FGameplayTag::RequestGameplayTag(TagName);
	if (!AbilityTag.IsValid())
		return EBTNodeResult::Failed;

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Pawn);
	if (!ASC)
		return EBTNodeResult::Failed;

	FGameplayAbilitySpec* Spec = nullptr;
	for (FGameplayAbilitySpec& CurrentSpec : ASC->GetActivatableAbilities())
	{
		if (!CurrentSpec.Ability) continue;

		if (CurrentSpec.Ability->AbilityTags.HasTag(AbilityTag) ||
			CurrentSpec.Ability->GetAssetTags().HasTag(AbilityTag))
		{
			Spec = &CurrentSpec;
			break;
		}
	}

	if (!Spec || !Spec->Ability)
		return EBTNodeResult::Failed;

	USFGA_Enemy_BaseAttack* Ability = Cast<USFGA_Enemy_BaseAttack>(Spec->Ability);
	if (!Ability)
		return EBTNodeResult::Failed;

	if (Ability->GetAttackType() == EAttackType::Range)
		return EBTNodeResult::Succeeded;

	const float* AttackRangePtr = Spec->SetByCallerTagMagnitudes.Find(SFGameplayTags::Data_EnemyAbility_AttackRange);
	CachedAttackRange = AttackRangePtr ? *AttackRangePtr : 200.f;

	float CurrentDistance = BB->GetValueAsFloat(DistanceKey.SelectedKeyName);
	if (CurrentDistance <= CachedAttackRange * RangeMultiplier)
		return EBTNodeResult::Succeeded;

	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!Target)
		return EBTNodeResult::Failed;

	AIController->MoveToActor(Target, CachedAttackRange * 0.8f);
	bIsMoving = true;
	ElapsedTime = 0.0f;

	return EBTNodeResult::InProgress;
}

void USFBTTask_MoveToAbilityRange::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	if (!bIsMoving)
		return;

	ElapsedTime += DeltaSeconds;

	if (ElapsedTime > MaxDuration)
	{
		AAIController* AIController = OwnerComp.GetAIOwner();
		if (AIController)
			AIController->StopMovement();

		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	float CurrentDistance = BB->GetValueAsFloat(DistanceKey.SelectedKeyName);
	if (CurrentDistance <= CachedAttackRange * RangeMultiplier)
	{
		AAIController* AIController = OwnerComp.GetAIOwner();
		if (AIController)
			AIController->StopMovement();

		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

void USFBTTask_MoveToAbilityRange::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController && bIsMoving)
		AIController->StopMovement();

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (BB)
	{
		if (TaskResult == EBTNodeResult::Failed)
		{
			int32 FailCount = BB->GetValueAsInt(MoveFailCountKey.SelectedKeyName);
			BB->SetValueAsInt(MoveFailCountKey.SelectedKeyName, FailCount + 1);
			BB->ClearValue(SelectedAbilityTagKey.SelectedKeyName);
		}
		else if (TaskResult == EBTNodeResult::Succeeded)
		{
			BB->SetValueAsInt(MoveFailCountKey.SelectedKeyName, 0);
		}
	}

	bIsMoving = false;
	ElapsedTime = 0.0f;
	CachedAttackRange = 0.0f;

	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}
