#pragma once
#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "AI/StateMachine/State/SFState.h"
#include "GameplayTagContainer.h"
#include "BossPhaseState.generated.h"

class USFGameplayAbility;
struct FGameplayAbilitySpecHandle;


USTRUCT(BlueprintType)
struct FSFBossPhaseAbility
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TSubclassOf<USFGameplayAbility> AbilityClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	bool bActivateOnEnter = false;

	UPROPERTY(EditDefaultsOnly)
	bool bClearOnExit = true;
};

USTRUCT()
struct FGrantedPhaseAbility
{
	GENERATED_BODY()

	FGameplayAbilitySpecHandle Handle;
	bool bClearOnExit;
};


UCLASS()
class SF_API UBossPhaseState : public USFState
{
	GENERATED_BODY()

public:
	virtual void OnEnter_Implementation() override;
	virtual void OnExit_Implementation() override;

protected:
	void GivePhaseAbilities();
	void ClearPhaseAbilities();

protected:
	
	UPROPERTY(EditDefaultsOnly, Category = "SF|BossPhaseAbility")
	TArray<FSFBossPhaseAbility> PhaseAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "SF|BossPhaseBT")
	FGameplayTag BehaviourTag;
    
	UPROPERTY()
	TArray<FGrantedPhaseAbility> GrantedPhaseAbilityHandles;
};