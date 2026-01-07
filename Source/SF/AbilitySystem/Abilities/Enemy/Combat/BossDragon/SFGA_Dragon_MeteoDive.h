#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Enemy/Combat/SFGA_Enemy_BaseAttack.h"
#include "SFGA_Dragon_MeteoDive.generated.h"

class UAbilityTask_ApplyRootMotionMoveToForce;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitDelay;

UCLASS()
class SF_API USFGA_Dragon_MeteorDive : public USFGA_Enemy_BaseAttack
{
    GENERATED_BODY()

public:
    USFGA_Dragon_MeteorDive();

protected:
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
    
    UFUNCTION() 
    void OnHoverTimeFinished();
    
    void LaunchDiveAttack();
    
    UFUNCTION()
    void OnDiveFinished();
    
    void ApplyImpactDamage(const FVector& ImpactLocation);

    UFUNCTION()
    void UpdateRotationToTarget();

protected:
    UPROPERTY(EditDefaultsOnly, Category = "SF|Dive|Preparation")
    float HoverDuration = 1.0f; 
    
    UPROPERTY(EditDefaultsOnly, Category = "SF|Dive|Dive")
    TObjectPtr<UAnimMontage> DiveLoopMontage; 

    UPROPERTY(EditDefaultsOnly, Category = "SF|Dive|Dive")
    float DiveSpeed = 6000.0f; 
    
    UPROPERTY(EditDefaultsOnly, Category = "SF|Dive|Dive")
    FVector MapCenterLocation = FVector(0.0f, 0.0f, 100.0f);

    UPROPERTY(EditDefaultsOnly, Category = "SF|Dive|Impact")
    float ImpactRadius = 2000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "SF|Dive|Debug")
    bool bShowDebugSphere = false;

private:
    FVector TargetLandLocation;

    FTimerHandle RotationTimerHandle;
};