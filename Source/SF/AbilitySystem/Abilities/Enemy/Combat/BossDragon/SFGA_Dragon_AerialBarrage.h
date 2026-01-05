#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/SFGameplayAbility.h"
#include "AbilitySystem/Abilities/Enemy/Combat/SFGA_Enemy_BaseAttack.h"
#include "SFGA_Dragon_AerialBarrage.generated.h"

class ASFCharacterBase;
class UAbilityTask_ApplyRootMotionConstantForce; // 전방 선언

UCLASS()
class SF_API USFGA_Dragon_AerialBarrage : public USFGA_Enemy_BaseAttack
{
    GENERATED_BODY()

public:
    USFGA_Dragon_AerialBarrage();

protected:
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, 
                                const FGameplayAbilityActorInfo* ActorInfo, 
                                const FGameplayAbilityActivationInfo ActivationInfo, 
                                const FGameplayEventData* TriggerEventData) override;
                                
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, 
                           const FGameplayAbilityActorInfo* ActorInfo, 
                           const FGameplayAbilityActivationInfo ActivationInfo, 
                           bool bReplicateEndAbility, 
                           bool bWasCancelled) override;

    void StartAscend();
    void StartOrbitAttack();

    UFUNCTION() void TickOrbitMovement();
    UFUNCTION() void OnFireballEventReceived(FGameplayEventData Payload);
    UFUNCTION() void OnMontageEnded();

    void SelectRandomLivingTarget();
    
    // 연계 관련
    void TryChainToDive();
    UFUNCTION() void OnChainAbilityEnded();

    virtual float CalcAIScore(const FEnemyAbilitySelectContext& Context) const override;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "SF|Barrage")
    float TargetAltitude = 2500.f;

    UPROPERTY(EditDefaultsOnly, Category = "SF|Barrage")
    float AscendSpeed = 1500.f;

    UPROPERTY(EditDefaultsOnly, Category = "SF|Barrage")
    float OrbitRadius = 2000.f;

    UPROPERTY(EditDefaultsOnly, Category = "SF|Barrage")
    FVector MapCenterLocation = FVector::ZeroVector;

    UPROPERTY(EditDefaultsOnly, Category = "SF|Barrage")
    int32 MaxFireballCount = 12;

    UPROPERTY(EditDefaultsOnly, Category = "SF|Barrage")
    int32 ShotsPerMontageLoop = 3;

    UPROPERTY(EditDefaultsOnly, Category = "SF|Barrage")
    TObjectPtr<UAnimMontage> TakeOffMontage;

    UPROPERTY(EditDefaultsOnly, Category = "SF|Barrage")
    TObjectPtr<UAnimMontage> FlyingAttackMontage;

    UPROPERTY(EditDefaultsOnly, Category = "SF|Barrage")
    TSubclassOf<AActor> ProjectileClass;

    UPROPERTY(EditDefaultsOnly, Category = "SF|Barrage")
    FName MuzzleSocketName = TEXT("Muzzle_Front");

private:
    TArray<ASFCharacterBase*> PlayerList;
    TWeakObjectPtr<AActor> TargetActor;
    FTimerHandle OrbitTimerHandle;
    FTimerHandle AscendCheckTimerHandle;

    // 루트 모션 제어용
    UPROPERTY()
    TObjectPtr<UAbilityTask_ApplyRootMotionConstantForce> AscendTask;

    // 물리 마찰력 복구용 캐시
    float CachedBrakingDeceleration = 0.0f;

    int32 CurrentFireballCount = 0;
    float CalculatedOrbitSpeed = 1200.f;

    bool bIsClockwise = false; 
};