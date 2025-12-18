// SFUpgradeBonusData.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "SFUpgradeBonusData.generated.h"

class UGameplayEffect;
class UGameplayAbility;

/**
 * 티어별 영구 업그레이드 보너스 정의
 * - DataTable Row
 * - Category + Tier 조합으로 1행
 */
USTRUCT(BlueprintType)
struct SF_API FSFUpgradeTierBonus : public FTableRowBase
{
	GENERATED_BODY()

	// 티어 (0 / 1 / 2 / 3)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Tier = 0; // 0이면 "레벨당 보너스", 1~은 티어 보너스

	// UI 표시용 설명
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	// ========================= 스탯 보너스 =========================
	// ※ 대부분 SetByCaller로 GE에 전달됨
	
	//최대 체력 증가 %
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat Bonus")
	float BonusMaxHealthPercent = 0.f;
	//최대 마나 증가 %
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat Bonus")
	float BonusMaxManaPercent = 0.f;
	//최대 스태미나 증가 %
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat Bonus")
	float BonusMaxStaminaPercent = 0.f;
	//치명타 피해 증가 %
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat Bonus")
	float BonusCriticalDamagePercent = 0.f;
	//공격력 증가 %
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat Bonus")
	float BonusAttackPowerPercent = 0.f;

	//최대 체력 증가
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat Bonus")
	float BonusMaxHealth = 0.f;
	//최대 마나 증가
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat Bonus")
	float BonusMaxMana = 0.f;
	//최대 스태미나 증가
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat Bonus")
	float BonusMaxStamina = 0.f;
	//행운 증가
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat Bonus")
	float BonusLuck = 0.f;
	//치명타 확률 증가
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat Bonus")
	float BonusCriticalChance = 0.f;
	
	// 쿨타임 감소 %
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat Bonus")
	float BonusCooldownReductionPercent = 0.f;
	// 구르기 무적 시간 추가 (초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat Bonus")
	float BonusDodgeInvincibilityTime = 0.f;
	// 재화 획득량 증가 %
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat Bonus")
	float BonusCurrencyGainPercent = 0.f;

	// ========================= 특수 효과 =========================

	// 달성 시 ASC에 부여되는 태그 (패시브 조건 체크용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Special Effect")
	FGameplayTagContainer GrantedTags;

	// 달성 시 부여되는 패시브 어빌리티
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Special Effect")
	TArray<TSubclassOf<UGameplayAbility>> GrantedAbilities;

	// 추가로 적용할 GameplayEffect (부활, 쉴드 등 복합 효과)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Special Effect")
	TSubclassOf<UGameplayEffect> BonusEffect;
};
