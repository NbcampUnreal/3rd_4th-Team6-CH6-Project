// SFGA_Dragon_Charge.h

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Enemy/Combat/SFGA_Enemy_BaseAttack.h"
#include "SFGA_Dragon_Charge.generated.h"

UCLASS()
class SF_API USFGA_Dragon_Charge : public USFGA_Enemy_BaseAttack
{
    GENERATED_BODY()

public:
    USFGA_Dragon_Charge();

protected:
    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData
    ) override;

    virtual void EndAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEndAbility,
        bool bWasCancelled
    ) override;

    virtual bool CanActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayTagContainer* SourceTags = nullptr,
        const FGameplayTagContainer* TargetTags = nullptr,
        FGameplayTagContainer* OptionalRelevantTags = nullptr
    ) const override;

    void FinishCharge(bool bCancelled);

    UFUNCTION()
    void OnChargeFinished();

    UFUNCTION()
    void OnMontageEnded();
    
    UFUNCTION()
    void OnChargeOverlap(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );
    
    virtual float CalcScoreModifier(const FEnemyAbilitySelectContext& Context) const override;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Dragon|Charge")
    TObjectPtr<UAnimMontage> ChargeMontage;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Dragon|Charge")
    float ChargeSpeed = 3000.f;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Dragon|Charge")
    float ChargeDuration = 1.5f;

    TEnumAsByte<ECollisionResponse> OriginalPawnResponse;

    UPROPERTY()
    TSet<TWeakObjectPtr<AActor>> HitActors;

private:
    
    UPROPERTY()
    TObjectPtr<class UAbilityTask_PlayMontageAndWait> MontageTaskRef;
};