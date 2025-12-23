// Fill out your copyright notice in the Description page of Project Settings.

#include "SFBTTask_ApproachAndAttack.h"
#include "AIController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "Character/SFCharacterGameplayTags.h" // 프로젝트의 태그 헤더 (필요시 경로 수정)

// [중요] 이 헤더가 있어야 EPathFollowingRequestResult 에러가 안 납니다.
#include "Navigation/PathFollowingComponent.h"

USFBTTask_ApproachAndAttack::USFBTTask_ApproachAndAttack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = "SF Approach And Attack";
	bNotifyTick = true; // 거리 체크를 위해 Tick 활성화 필수
	bNotifyTaskFinished = true;
	bCreateNodeInstance = true; // 노드별로 독립적인 사거리를 갖기 위해 필수
}

UAbilitySystemComponent* USFBTTask_ApproachAndAttack::GetASC(UBehaviorTreeComponent& OwnerComp) const
{
	if (AAIController* AIController = OwnerComp.GetAIOwner())
	{
		if (APawn* Pawn = AIController->GetPawn())
		{
			return UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn);
		}
	}
	return nullptr;
}

EBTNodeResult::Type USFBTTask_ApproachAndAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* OwnerPawn = AIController ? AIController->GetPawn() : nullptr;
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();

	// 필수 데이터 체크
	if (!AIController || !OwnerPawn || !BB || !AbilityClassToRun)
	{
		return EBTNodeResult::Failed;
	}

	// 변수 초기화
	CachedOwnerComp = &OwnerComp;
	bFinished = false;
	bIsMoving = false;
	bIsAttacking = false;
	ElapsedTime = 0.0f;

	// 타겟 확인
	AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!TargetActor)
	{
		return EBTNodeResult::Failed;
	}

	// 1. 거리 체크: 이미 사거리 안인가?
	float DistSq = FVector::DistSquared(OwnerPawn->GetActorLocation(), TargetActor->GetActorLocation());
	float AttackRadiusSq = AttackRadius * AttackRadius;

	if (DistSq <= AttackRadiusSq)
	{
		// 이미 사거리 안이면 바로 공격 실행
		PerformAttack(OwnerComp);
	}
	else
	{
		// 2. 사거리 밖이면 이동 시작
		bIsMoving = true;
		
		// MoveToActor: 타겟을 향해 이동 (StopOnOverlap false, 우리가 직접 멈춤 제어)
		EPathFollowingRequestResult::Type Result = AIController->MoveToActor(TargetActor, AttackRadius * 0.5f, true, true, false, 0, true);
		
		if (Result == EPathFollowingRequestResult::Failed)
		{
			return EBTNodeResult::Failed;
		}
	}

	return EBTNodeResult::InProgress;
}

void USFBTTask_ApproachAndAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	if (bFinished) return;

	ElapsedTime += DeltaSeconds;

	// [안전장치] 타임아웃
	if (MaxDuration > 0.0f && ElapsedTime >= MaxDuration)
	{
		bFinished = true;
		if (bIsMoving)
		{
			if (AAIController* AIController = OwnerComp.GetAIOwner())
			{
				AIController->StopMovement();
			}
		}
		CleanupDelegate(OwnerComp);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded); // 상황에 따라 Failed 처리 가능
		return;
	}

	// [핵심] 이동 중일 때 실시간 거리 체크
	if (bIsMoving)
	{
		AAIController* AIController = OwnerComp.GetAIOwner();
		APawn* OwnerPawn = AIController ? AIController->GetPawn() : nullptr;
		UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
		AActor* TargetActor = BB ? Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName)) : nullptr;

		if (OwnerPawn && TargetActor)
		{
			float DistSq = FVector::DistSquared(OwnerPawn->GetActorLocation(), TargetActor->GetActorLocation());
			float AttackRadiusSq = AttackRadius * AttackRadius;

			// 설정한 사거리(AttackRadius) 안으로 들어왔다면?
			if (DistSq <= AttackRadiusSq)
			{
				bIsMoving = false; // 이동 모드 해제
				
				// 즉시 멈춤
				if (AIController)
				{
					AIController->StopMovement();
				}

				// 공격 시작
				PerformAttack(OwnerComp);
			}
		}
		else
		{
			// 타겟이나 폰이 사라지면 실패
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		}
	}
}

void USFBTTask_ApproachAndAttack::PerformAttack(UBehaviorTreeComponent& OwnerComp)
{
	UAbilitySystemComponent* ASC = GetASC(OwnerComp);
	if (!ASC)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// 지정한 클래스와 일치하는 어빌리티 스펙 찾기
	FGameplayAbilitySpec* FoundSpec = nullptr;
	for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (Spec.Ability && Spec.Ability->GetClass() == AbilityClassToRun)
		{
			FoundSpec = const_cast<FGameplayAbilitySpec*>(&Spec);
			break;
		}
	}

	// 어빌리티 실행 시도
	if (!FoundSpec || !ASC->TryActivateAbility(FoundSpec->Handle))
	{
		// 쿨타임, 비용 부족 등으로 실행 실패 시
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	bIsAttacking = true;

	// 기다릴 태그 결정 (설정값 없으면 기본값 사용)
	// 주의: SFGameplayTags 헤더가 없으면 아래 줄에서 에러 날 수 있으니 
	// 필요하면 FGameplayTag::RequestGameplayTag("Character.State.Attacking") 형태로 쓰세요.
	TagToWait = WaitForTag.IsValid() ? WaitForTag : SFGameplayTags::Character_State_Attacking;

	// 태그 감지 시작 (공격 모션이 끝날 때까지 대기)
	if (ASC->GetTagCount(TagToWait) > 0)
	{
		if (!EventHandle.IsValid())
		{
			EventHandle = ASC->RegisterGameplayTagEvent(TagToWait, EGameplayTagEventType::NewOrRemoved)
				.AddUObject(this, &USFBTTask_ApproachAndAttack::OnTagChanged);
		}
	}
	else
	{
		// 태그가 즉시 사라졌거나 안 붙음 -> 성공 처리
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

void USFBTTask_ApproachAndAttack::OnTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (bFinished) return;

	// 태그가 사라지면(0) 공격 끝 -> 태스크 성공 종료
	if (NewCount == 0)
	{
		bFinished = true;
		if (UBehaviorTreeComponent* OwnerComp = CachedOwnerComp.Get())
		{
			CleanupDelegate(*OwnerComp);
			FinishLatentTask(*OwnerComp, EBTNodeResult::Succeeded);
		}
	}
}

void USFBTTask_ApproachAndAttack::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	CleanupDelegate(OwnerComp);
	CachedOwnerComp.Reset();
	bIsMoving = false;
	bIsAttacking = false;
	
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

void USFBTTask_ApproachAndAttack::CleanupDelegate(UBehaviorTreeComponent& OwnerComp)
{
	if (EventHandle.IsValid())
	{
		if (UAbilitySystemComponent* ASC = GetASC(OwnerComp))
		{
			ASC->UnregisterGameplayTagEvent(EventHandle, TagToWait, EGameplayTagEventType::NewOrRemoved);
		}
	}
	EventHandle.Reset();
}