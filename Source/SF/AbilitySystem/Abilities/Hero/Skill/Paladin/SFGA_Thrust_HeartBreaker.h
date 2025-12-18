#pragma once

#include "CoreMinimal.h"
#include "SFGA_Thrust_Base.h"
#include "SFGA_Thrust_HeartBreaker.generated.h"

USTRUCT(BlueprintType)
struct FSFChargePhaseInfo
{
	GENERATED_BODY()

	// 이 Phase에서 다음 Phase로 넘어가는 시간 (마지막 Phase에서는 무시됨)
	UPROPERTY(EditDefaultsOnly, Category="Charging", meta=(ClampMin="0.0"))
	float ChargeTimeToNext = 0.5f;

	// UI 표시 색상
	UPROPERTY(EditDefaultsOnly, Category="UI")
	FLinearColor PhaseColor = FLinearColor::White;

	// 데미지 배율
	UPROPERTY(EditDefaultsOnly, Category="Combat", meta=(ClampMin="0.0"))
	float DamageMultiplier = 1.f;

	// 돌진 거리 배율
	UPROPERTY(EditDefaultsOnly, Category="Combat", meta=(ClampMin="0.0"))
	float RushDistanceScale = 1.f;

	// 공격 시 회전 보간 속도
	UPROPERTY(EditDefaultsOnly, Category="Combat", meta=(ClampMin="0.0"))
	float AttackInterpSpeed = 15.f;

	// 슬라이딩 모드
	UPROPERTY(EditDefaultsOnly, Category="Movement")
	ESFSlidingMode SlidingMode = ESFSlidingMode::Normal;

	// 카메라 모드
	UPROPERTY(EditDefaultsOnly, Category="Camera")
	TSubclassOf<USFCameraMode> CameraMode;

	// 돌진 몽타주
	UPROPERTY(EditDefaultsOnly, Category="Animation")
	TObjectPtr<UAnimMontage> RushMontage;
};


/**
 * 
 */
UCLASS()
class SF_API USFGA_Thrust_HeartBreaker : public USFGA_Thrust_Base
{
	GENERATED_BODY()

public:
	USFGA_Thrust_HeartBreaker(FObjectInitializer const& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void OnTrace(FGameplayEventData Payload) override;

	float GetPhaseDamage() const;
	float GetPhaseRushDistance() const;
	float GetPhaseAttackInterpSpeed() const;
	ESFSlidingMode GetPhaseSlidingMode() const;
	UAnimMontage* GetPhaseRushMontage() const;
	TSubclassOf<USFCameraMode> GetPhaseCameraMode() const;
	FLinearColor GetPhaseColor() const;
	
	UFUNCTION()
	void OnKeyReleased(float TimeHeld);
	
	void OnServerTargetDataReceivedCallback(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag ActivationTag);

	void ExecuteRushAttack();

	int32 CalculatePhase(float TimeHeld) const;
	
	UFUNCTION()
	void OnRushMontageFinished();

	void StartPhaseTimer();
	void OnPhaseTimePassed();
	void ResetCharge();

	// UI 초기화 및 숨김 처리 (bShow=true: 표시, bShow=false: 숨김)
	void BroadcastUIConstruct(bool bShow);
	// 페이즈 변경 시 색상 업데이트 메시지 전송
	void BroadcastUIRefresh(int32 NewPhaseIndex);

	// 차징 Cue 시작
	void StartChargingCue();
	
	// 차징 Cue Phase 갱신
	void UpdateChargingCuePhase();
	
	// 차징 Cue 종료 
	void StopChargingCue();

	void UpdateCameraModeForPhase(int32 PhaseIndex);

protected:

	// Phase 별 데이터 설정
	UPROPERTY(EditDefaultsOnly, Category="SF|PhaseInfo")
	TArray<FSFChargePhaseInfo> PhaseInfos;

	// 차징 몽타주 (Start → Loop)
	UPROPERTY(EditDefaultsOnly, Category="SF|Animation")
	TObjectPtr<UAnimMontage> ChargingMontage;
	
	// 차징 중 회전 보간 속도 (느리게)
	UPROPERTY(EditDefaultsOnly, Category="SF|Startup")
	float ChargingInterpSpeed = 3.f;

	UPROPERTY(EditDefaultsOnly, Category="SF|Startup")
	float BaseRushDistance = 124.f;
	
	// 슈퍼아머 GE
	UPROPERTY(EditDefaultsOnly, Category="SF|Buff")
	TSubclassOf<UGameplayEffect> SuperArmorEffectClass;

	UPROPERTY(EditDefaultsOnly, Category="SF|GameplayCue")
	FGameplayTag ChargingCueTag;
	
private:

	FActiveGameplayEffectHandle SuperArmorEffectHandle;

	// 현재 차징 페이즈(서버는 클라값 신뢰/검증 후 적용)
	int32 CurrentPhaseIndex = 0;
	int32 MaxPhaseIndex = 0;
	
	// 총 차징 시간
	float TotalChargeTime = 0.f;

	// 서버 검증용: 어빌리티 시작 시간
	float AbilityStartTime = 0.f;

	FTimerHandle PhaseTimerHandle;

	FDelegateHandle ServerTargetDataDelegateHandle;
};
