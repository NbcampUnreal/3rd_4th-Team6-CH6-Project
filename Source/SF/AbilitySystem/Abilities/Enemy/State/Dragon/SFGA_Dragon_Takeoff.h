#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/SFGameplayAbility.h"
#include "SFGA_Dragon_TakeOff.generated.h"

class UCurveFloat;

UCLASS()
class SF_API USFGA_Dragon_TakeOff : public USFGameplayAbility
{
	GENERATED_BODY()

public:
	USFGA_Dragon_TakeOff();

	UFUNCTION()
	void EndAbilityDefault();
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UPROPERTY(EditAnywhere, Category = "TakeOff")
	TObjectPtr<UAnimMontage> TakeOffMontage;
};