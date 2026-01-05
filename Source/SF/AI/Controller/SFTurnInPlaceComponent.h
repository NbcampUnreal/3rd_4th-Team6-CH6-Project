#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SFTurnInPlaceComponent.generated.h"

class ASFBaseAIController;
class UAbilitySystemComponent;

UCLASS(ClassGroup=(AI), meta=(BlueprintSpawnableComponent))
class SF_API USFTurnInPlaceComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USFTurnInPlaceComponent();

    virtual void InitializeTurnInPlaceComponent();

    UFUNCTION(BlueprintCallable, Category="Turn In Place")
    void RequestTurnToTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category="Turn In Place")
    void OnTurnFinished();

    UFUNCTION(BlueprintPure, Category="Turn In Place")
    bool IsTurning() const { return bIsTurning; }

    UFUNCTION(BlueprintPure, Category="Turn In Place")
    float GetAngleToTarget() const;

    UFUNCTION(BlueprintPure, Category="Turn In Place")
    float GetTurnThreshold() const { return TurnThreshold; }

    UFUNCTION(BlueprintCallable, Category="Turn In Place")
    void SyncControlRotationToTarget();

    void DisableSmoothRotation();
    bool CanTurnInPlace() const;

private:
    void ExecuteTurn(float DeltaYaw);

    ASFBaseAIController* GetAIController() const;
    APawn* GetControlledPawn() const;
    UAbilitySystemComponent* GetASC() const;
    AActor* GetTargetActor() const;

private:
    UPROPERTY(EditAnywhere, Category="Turn In Place")
    float TurnThreshold = 45.f;

    UPROPERTY(EditAnywhere, Category="Turn In Place")
    float LargeTurnThreshold = 135.f;

    UPROPERTY(EditAnywhere, Category="Turn In Place")
    float RotationInterpSpeed = 7.0f;

    UPROPERTY(EditAnywhere, Category="Turn In Place")
    float AcceptableAngle = 5.0f;

    bool bIsTurning = false;
    float LockedDeltaYaw = 0.f;
};