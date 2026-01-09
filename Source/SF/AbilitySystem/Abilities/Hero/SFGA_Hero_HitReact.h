#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/SFGameplayAbility.h"
#include "SFGA_Hero_HitReact.generated.h"

/**
 * 
 */
UCLASS()
class SF_API USFGA_Hero_HitReact : public USFGameplayAbility
{
	GENERATED_BODY()

public:
	USFGA_Hero_HitReact(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	// 피격 각도 계산 (공격자 방향 → 캐릭터 로컬 각도) 
	float CalculateHitAngle(const FGameplayEventData* EventData) const;

	// 커스텀 각도 기준으로 방향별 몽타주 선택 
	FSFMontagePlayData SelectHitReactMontage(float HitAngle) const;
    
	UFUNCTION()
	void OnNetSync();
    
	UFUNCTION()
	void OnMontageFinished();

protected:

	UPROPERTY(EditDefaultsOnly, Category = "SF|HitReact")
	FGameplayTag HitReactMontageTag;

	// 정면 피격 판정 각도 (|Yaw| < 이 값이면 정면) 
	UPROPERTY(EditDefaultsOnly, Category = "SF|HitReact", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float FrontAngleThreshold = 50.f;
    
	// 후면 피격 판정 각도 (|Yaw| > 이 값이면 후면)
	UPROPERTY(EditDefaultsOnly, Category = "SF|HitReact", meta = (ClampMin = "90.0", ClampMax = "180.0"))
	float BackAngleThreshold = 130.f;

	UPROPERTY(EditDefaultsOnly, Category = "SF|HitReact")
	TSubclassOf<UCameraShakeBase> HitReactCameraShakeClass;


	// 피격 시 미세 밀림 활성화 *
	UPROPERTY(EditDefaultsOnly, Category = "SF|HitReact")
	bool bApplyMicroKnockback = true;
    
	// 미세 밀림 강도 
	UPROPERTY(EditDefaultsOnly, Category = "SF|HitReact", meta = (EditCondition = "bApplyMicroKnockback", ClampMin = "0.0"))
	float MicroKnockbackStrength = 150.f;
    
	// 미세 밀림 지속 시간 
	UPROPERTY(EditDefaultsOnly, Category = "SF|HitReact", meta = (EditCondition = "bApplyMicroKnockback", ClampMin = "0.0"))
	float MicroKnockbackDuration = 0.3f;
    
	// 미세 밀림 강도 커브 (시간에 따른 감쇠) 
	UPROPERTY(EditDefaultsOnly, Category = "SF|HitReact", meta = (EditCondition = "bApplyMicroKnockback"))
	TObjectPtr<UCurveFloat> MicroKnockbackStrengthCurve;

private:
	// 몽타주 데이터 (ActivateAbility → OnNetSync 전달용) 
	FSFMontagePlayData CachedMontageData;

	TWeakObjectPtr<AActor> CachedInstigator;
};
