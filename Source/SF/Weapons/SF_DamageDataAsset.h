#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "SF_DamageDataAsset.generated.h"

class UGameplayEffect;

/**
 * 무기 스킬의 대미지 데이터를 관리하는 데이터 에셋입니다.
 * 예: 검 찌르기의 대미지, 크리티컬 확률 등을 한 곳에서 관리
 */
UCLASS(BlueprintType)
class SF_API USF_DamageDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// --- 기본 대미지 ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	float BaseDamage = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	float CriticalDamageMultiplier = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	float CriticalChance = 0.2f; // 20%

	// --- 게임플레이 이펙트 ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	TSubclassOf<class UGameplayEffect> DamageEffectClass;

	// --- 태그 설정 ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	FGameplayTag DamageEventTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	FGameplayTag SetByCallerTag = FGameplayTag::RequestGameplayTag(FName("Damage"));

	// --- 트레이스 설정 ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace")
	float TraceDistance = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace")
	float TraceRadius = 50.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace")
	float TraceVisualizationDuration = 2.0f;

	// --- 헬퍼 함수 ---
	UFUNCTION(BlueprintCallable, Category = "Damage")
	float GetFinalDamage(bool bIsCritical = false) const
	{
		return bIsCritical ? BaseDamage * CriticalDamageMultiplier : BaseDamage;
	}

	UFUNCTION(BlueprintCallable, Category = "Damage")
	bool IsHitCritical() const
	{
		return FMath::FRand() < CriticalChance;
	}
};