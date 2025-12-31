// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SFCombatComponentBase.h"
#include "Perception/AIPerceptionTypes.h"
#include "SFEnemyCombatComponent.generated.h"

class USFCombatSet_Enemy;


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SF_API USFEnemyCombatComponent : public USFCombatComponentBase
{
    GENERATED_BODY()

public:
    USFEnemyCombatComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    UFUNCTION(BlueprintPure, Category = "SF|Combat")
    static USFEnemyCombatComponent* FindSFEnemyCombatComponent(const AController* Controller)
    {
        return (Controller ? Controller->FindComponentByClass<USFEnemyCombatComponent>() : nullptr);
    }

    
    virtual void InitializeCombatComponent() override;
    
    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

protected:
    
    virtual void EvaluateTarget() override;
    
    float CalculateTargetScore(AActor* Target) const;

    
    void UpdateCombatRangeTags();

    
    void UpdatePerceptionConfig();

    
    void StartTargetEvaluationTimer();

    
    void StopTargetEvaluationTimer();

    
    UFUNCTION()
    void OnTargetEvaluationTimer();

protected:
    
    UPROPERTY()
    TObjectPtr<USFCombatSet_Enemy> CachedCombatSet;

    // Target evaluation interval
    UPROPERTY(EditAnywhere, Category = "Combat|Target")
    float TargetEvaluationInterval = 0.5f;

    // Timer handle for periodic evaluation
    FTimerHandle TargetEvaluationTimerHandle;
};