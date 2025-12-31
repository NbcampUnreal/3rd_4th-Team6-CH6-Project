#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SFTurnInPlaceComponent.generated.h"

class ASFBaseAIController;
class UAbilitySystemComponent;

/**
 * 드래곤 AI의 TurnInPlace 기능을 담당하는 컴포넌트
 * - 큰 각도 회전: TurnInPlace Ability 실행 (애니메이션 연출)
 * - 작은 각도 회전: 스무스 회전 (CharacterMovement)
 * - Service와 함께 사용하여 자동으로 타겟 방향 유지
 */
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


    void EnableSmoothRotation();
    void DisableSmoothRotation();

private:

    void ExecuteTurn(float DeltaYaw);

   
    bool CanTurnInPlace() const;


    ASFBaseAIController* GetAIController() const;
    APawn* GetControlledPawn() const;
    UAbilitySystemComponent* GetASC() const;
    AActor* GetTargetActor() const;

private:

    UPROPERTY(EditAnywhere, Category="Turn In Place", meta=(ClampMin="0.0", ClampMax="180.0"))
    float TurnThreshold = 45.f;


    UPROPERTY(EditAnywhere, Category="Turn In Place", meta=(ClampMin="0.0", ClampMax="180.0"))
    float LargeTurnThreshold = 135.f;


    bool bIsTurning = false;

 
    float LockedDeltaYaw = 0.f;
};