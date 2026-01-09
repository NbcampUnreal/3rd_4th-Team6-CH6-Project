#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "SFAbilityTask_RotateWithCurve.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRotateWithCurveDelegate);

UCLASS()
class SF_API USFAbilityTask_RotateWithCurve : public UAbilityTask
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FRotateWithCurveDelegate OnCompleted;

	UPROPERTY(BlueprintAssignable)
	FRotateWithCurveDelegate OnCancelled;

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static USFAbilityTask_RotateWithCurve* CreateRotateWithCurveTask(
		UGameplayAbility* OwningAbility,
		FName TaskInstanceName,
		float InMontageDuration,
		float InTargetYaw,
		float InActualTurnYaw);

	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;
	virtual void OnDestroy(bool bInOwnerFinished) override;

private:
	float MontageDuration;
	float TargetYaw;
	float ActualTurnYaw;
	float InitialYaw;
	float TimeElapsed;
	bool bIsFinished;
};