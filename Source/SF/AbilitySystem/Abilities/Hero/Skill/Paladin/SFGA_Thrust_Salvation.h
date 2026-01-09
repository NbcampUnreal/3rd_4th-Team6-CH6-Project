#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Hero/Skill/SFGA_Skill_Melee.h"

#include "SFGA_Thrust_Salvation.generated.h"

class USFDA_BuffAuraEffectData;
class USFDA_WaveEffectData;
/**
 * 
 */
UCLASS()
class SF_API USFGA_Thrust_Salvation : public USFGA_Skill_Melee
{
	GENERATED_BODY()

public:
	USFGA_Thrust_Salvation(FObjectInitializer const& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	void ApplyDamageAndKnockback(ASFCharacterBase* Source, ASFCharacterBase* Target, UAbilitySystemComponent* TargetASC);
	void ApplyBuffToAlly(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC);

	// 파장 VFX + 사운드 + 아군 버프 적용
	void ApplyWaveEffectAndAllyBuff();
	
private:
	
	// 돌진 몽타주 완료 후 ShieldBash 몽타주로 전환
	UFUNCTION()
	void OnThrustMontageCompleted();
	
	// 캐릭터 전방에 캡슐 오버랩 검사 수행 및 넉백 효과를 위한 GameplayEvent_Knockback 이벤트 전송
	UFUNCTION()
	void OnShieldBashEffectBegin(FGameplayEventData Payload);

	UFUNCTION()
	void OnMontageFinished();

protected:

	UPROPERTY(EditDefaultsOnly, Category="SF|Animation")
	TObjectPtr<UAnimMontage> ThrustMontage;

	UPROPERTY(EditDefaultsOnly, Category="SF|Animation")
	TObjectPtr<UAnimMontage> ShieldBashMontage;

	/**
	 * 공격 범위 전방 오프셋 거리
	 * 캐릭터 위치에서 전방으로 이 거리만큼 떨어진 위치에서 오버랩 검사 수행
	 * 공격 중심 = 캐릭터 위치 + (전방 벡터 * AttackDistance)
	 */
	UPROPERTY(EditDefaultsOnly, Category="SF|Attack")
	float AttackDistance = 100.f;

	/**
	 * 공격 범위 반경 배율
	 * 캐릭터 캡슐 반경에 이 값을 곱하여 공격 범위 반경을 결정
	 * 공격 반경 = 캐릭터 캡슐 반경 * AttackRadiusMultiplier
	 */
	UPROPERTY(EditDefaultsOnly, Category="SF|Attack")
	float AttackRadiusMultiplier = 3.25f;

	// 아군 버프 GE (Defense↑, MoveSpeed↑)
	UPROPERTY(EditDefaultsOnly, Category="SF|AllyBuff")
	TSubclassOf<UGameplayEffect> BuffEffectClass;

	// 타원 전방 반경 (장축)
	UPROPERTY(EditDefaultsOnly, Category="SF|AllyBuff")
	float WaveForwardRadius = 500.f;

	// 타원 좌우 반경 (단축)
	UPROPERTY(EditDefaultsOnly, Category="SF|AllyBuff")
	float WaveSideRadius = 500.f;

	// 타원 높이 판정
	UPROPERTY(EditDefaultsOnly, Category="SF|AllyBuff")
	float WaveHalfHeight = 200.f; 
	
	// 자기 자신도 버프 받을지 여부
	UPROPERTY(EditDefaultsOnly, Category="SF|AllyBuff")
	bool bBuffSelf = true;
	
	// 발 밑 파장 VFX + 사운드 Cue 태그
	UPROPERTY(EditDefaultsOnly, Category="SF|Cue")
	FGameplayTag WaveEffectCueTag;
	UPROPERTY(EditDefaultsOnly, Category="SF|VFX")
	TObjectPtr<USFDA_WaveEffectData> WaveEffectData;

	UPROPERTY(EditDefaultsOnly, Category="SF|VFX")
	TObjectPtr<USFDA_BuffAuraEffectData> BuffAuraEffectData;

	// 돌진 구간 슬라이딩 모드
	UPROPERTY(EditDefaultsOnly, Category="SF|Movement")
	ESFSlidingMode ThrustSlidingMode = ESFSlidingMode::Normal;
    
	// ShieldBash 구간 슬라이딩 모드
	UPROPERTY(EditDefaultsOnly, Category="SF|Movement")
	ESFSlidingMode ShieldBashSlidingMode = ESFSlidingMode::Normal;
	
};
