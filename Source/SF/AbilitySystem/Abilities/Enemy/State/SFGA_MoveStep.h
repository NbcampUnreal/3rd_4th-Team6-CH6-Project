#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/SFGameplayAbility.h"
#include "SFGA_MoveStep.generated.h"

class UCurveFloat; // 전방 선언

UCLASS()
class SF_API USFGA_MoveStep : public USFGameplayAbility
{
	GENERATED_BODY()

public:
	USFGA_MoveStep();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:

	UPROPERTY(EditAnywhere, Category = "Movement")
	TObjectPtr<UAnimMontage> ForwardAnim; 

	UPROPERTY(EditAnywhere, Category = "Movement")
	TObjectPtr<UAnimMontage> BackwardAnim;
	
	UPROPERTY(EditAnywhere, Category = "Movement")
	TObjectPtr<UCurveFloat> StepCurve;
	
	UPROPERTY(EditAnywhere, Category = "Movement")
	float StepIntensity = 1500.f; 
	
	UPROPERTY(EditAnywhere, Category = "Movement")
	float StepDuration = 1.0f;

private:
	UFUNCTION()
	void OnMoveStepFinished();
};