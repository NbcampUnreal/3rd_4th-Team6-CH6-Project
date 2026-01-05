#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/SFGameplayAbility.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "SFGA_PhaseIntro.generated.h"

USTRUCT(BlueprintType)
struct FSFPhaseIntroStep
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UAnimMontage> Montage = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PlayRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName StartSection = NAME_None;

    /** 이 단계가 끝나고 다음 단계로 넘어가기 전 대기 시간 (선택 사항) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PostWaitTime = 0.0f;
};

UCLASS()
class SF_API USFGA_PhaseIntro : public USFGameplayAbility
{
    GENERATED_BODY()

public:
    USFGA_PhaseIntro();

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


    void PlayNextStep();
    
    UFUNCTION()
    void OnStepFinished();

protected:

    UPROPERTY(EditDefaultsOnly, Category = "SF|Phase Sequence")
    TArray<FSFPhaseIntroStep> IntroSteps;
    
    UPROPERTY(EditDefaultsOnly, Category = "SF|Phase Tags")
    FGameplayTagContainer PhaseLockTags;
    
    UPROPERTY(EditDefaultsOnly, Category = "SF|Phase Effects")
    TArray<TSubclassOf<UGameplayEffect>> PermanentEffects;
private:
    int32 CurrentStepIndex = 0;

    UPROPERTY(Transient)
    UAbilityTask_PlayMontageAndWait* CurrentMontageTask = nullptr;
    
    FTimerHandle StepTimerHandle;
};
