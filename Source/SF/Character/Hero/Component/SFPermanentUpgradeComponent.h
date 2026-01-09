#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"
#include "GameplayAbilitySpec.h"
#include "System/Data/SFPermanentUpgradeTypes.h"
#include "SFPermanentUpgradeComponent.generated.h"

class UAbilitySystemComponent;
class UDataTable;
class UGameplayEffect;
struct FSFUpgradeTierBonus;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SF_API USFPermanentUpgradeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USFPermanentUpgradeComponent();

	void ApplyUpgradeBonuses(UAbilitySystemComponent* ASC, const FSFPermanentUpgradeData& UpgradeData);
	void ClearUpgradeBonuses(UAbilitySystemComponent* ASC);

protected:
	virtual void BeginPlay() override;

private:
	// DataTable: RowName = "<category>_T0" / "<category>_T1" / "<category>_T2" / "<category>_T3" (e.g. "lust_T0")
	UPROPERTY(EditDefaultsOnly, Category="SF|PermanentUpgrade")
	UDataTable* UpgradeBonusTable = nullptr;

	// 누적 스탯을 SetByCaller로 전달하는 GE
	UPROPERTY(EditDefaultsOnly, Category="SF|PermanentUpgrade")
	TSubclassOf<UGameplayEffect> StatBonusEffectClass;

	// 적용된 것들 추적
	TWeakObjectPtr<UAbilitySystemComponent> AppliedASC;
	FActiveGameplayEffectHandle StatEffectHandle;
	TArray<FGameplayAbilitySpecHandle> GrantedAbilityHandles;
	FGameplayTagContainer GrantedLooseTags;
	TArray<FActiveGameplayEffectHandle> GrantedEffectHandles;

private:
	const FSFUpgradeTierBonus* FindTierRow(ESFUpgradeCategory Category, int32 Tier) const;
	static FString CategoryToKey(ESFUpgradeCategory Category);
};

