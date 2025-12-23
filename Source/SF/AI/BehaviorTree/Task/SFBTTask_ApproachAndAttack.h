// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "SFBTTask_ApproachAndAttack.generated.h"

class UAbilitySystemComponent;

/**
 * 타겟에게 접근하다가 사거리(AttackRadius) 내에 들어오면 즉시 멈추고 
 * 지정된 어빌리티(AbilityClassToRun)를 실행하는 태스크
 */
UCLASS()
class SF_API USFBTTask_ApproachAndAttack : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	USFBTTask_ApproachAndAttack(const FObjectInitializer& ObjectInitializer);

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

private:
	// 내부 헬퍼 함수
	UAbilitySystemComponent* GetASC(UBehaviorTreeComponent& OwnerComp) const;
	void PerformAttack(UBehaviorTreeComponent& OwnerComp);
	void OnTagChanged(const FGameplayTag Tag, int32 NewCount);
	void CleanupDelegate(UBehaviorTreeComponent& OwnerComp);

	// 내부 상태 변수
	bool bIsMoving = false;    // 이동 중인가?
	bool bIsAttacking = false; // 공격 후 대기 중인가?
	bool bFinished = false;    // 태스크 종료 플래그
	
	float ElapsedTime = 0.0f;
	FDelegateHandle EventHandle;
	TWeakObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;
	
	// 런타임에 결정되는 '기다릴 태그'
	FGameplayTag TagToWait; 

public:
	// [설정] 실행할 어빌리티 클래스 (내려찍기, 찌르기 등 BT에서 선택)
	UPROPERTY(EditAnywhere, Category = "Attack")
	TSubclassOf<UGameplayAbility> AbilityClassToRun;

	// [설정] 추적할 타겟 (블랙보드 키)
	UPROPERTY(EditAnywhere, Category = "Attack")
	FBlackboardKeySelector TargetActorKey;

	// [설정] 공격 사거리 (BT 에디터에서 직접 입력 가능)
	// 이 거리 안으로 들어오면 이동을 멈추고 공격합니다.
	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackRadius = 150.0f;

	// [옵션] 공격 상태 태그 (이 태그가 사라지면 공격이 끝난 것으로 간주. 기본값: Character.State.Attacking)
	UPROPERTY(EditAnywhere, Category = "Attack")
	FGameplayTag WaitForTag;

	// [안전장치] 최대 제한 시간 (이 시간이 지나면 강제 종료)
	UPROPERTY(EditAnywhere, Category = "Safety")
	float MaxDuration = 5.0f;
};