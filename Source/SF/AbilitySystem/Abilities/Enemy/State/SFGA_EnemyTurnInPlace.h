#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/SFGameplayAbility.h"
#include "GameplayTagContainer.h"
#include "SFGA_EnemyTurnInPlace.generated.h"

class USFAbilityTask_RotateWithCurve;
class UAbilityTask_PlayMontageAndWait;

/**
 * [Enemy Standard TurnInPlace]
 * - 모든 적(잡몹 포함)이 사용하는 표준 제자리 회전 어빌리티
 * - Root Motion을 사용하지 않고, Curve와 AbilityTask를 통해 직접 회전 제어
 * - "발 미끄러짐" 방지 및 정확한 회전 각도 보장
 */
UCLASS()
class SF_API USFGA_EnemyTurnInPlace : public USFGameplayAbility
{
	GENERATED_BODY()

public:
	USFGA_EnemyTurnInPlace();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

	// Task 콜백 함수
	UFUNCTION()
	void OnRotationTaskCompleted();

	UFUNCTION()
	void OnRotationTaskCancelled();

	UPROPERTY(EditDefaultsOnly, Category = "Turn In Place", meta = (Categories = "GameplayEvent.Turn"))
	TMap<FGameplayTag, UAnimMontage*> TurnMontageMap;

	UPROPERTY(EditDefaultsOnly, Category = "Turn In Place", meta = (ClampMin = "0.1", ClampMax = "3.0"))
	float MontagePlayRate = 1.0f;

	// 애니메이션 커브 이름 (모든 턴 애니메이션에 이 이름의 커브가 있어야 함)
	UPROPERTY(EditDefaultsOnly, Category = "Turn In Place")
	FName TurnCurveName = FName("RemainingTurnYaw");

protected:
	UPROPERTY()
	TObjectPtr<USFAbilityTask_RotateWithCurve> RotateTask;

	bool bIsRightTurn = false;
	float ActualTurnYaw = 0.0f;
	float TargetYaw = 0.0f;
	float InitialYaw = 0.0f;
	FGameplayTag TriggerEventTag;

	// 중복 종료 방지 플래그
	bool bIsEnding = false;

	// Helper Functions
	bool ValidateTriggerEvent(const FGameplayEventData* TriggerEventData);
	bool StartTurnMontage(const FGameplayTag& EventTag, float& OutDuration);
	void NotifyTurnFinished();
};