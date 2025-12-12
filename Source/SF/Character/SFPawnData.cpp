#include "SFPawnData.h"

USFPawnData::USFPawnData(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PawnClass = nullptr;
	InputConfig = nullptr;
	DefaultCameraMode = nullptr;
}

TArray<TSubclassOf<USFGameplayAbility>> USFPawnData::GetUpgradeOptionsForSlot(FGameplayTag InputTag) const
{
	if (const FSFSkillUpgradeOptionList* OptionList = SkillUpgradeMap.Find(InputTag))
	{
		return OptionList->UpgradeAbilities;
	}
	return TArray<TSubclassOf<USFGameplayAbility>>();
}

FGameplayTag USFPawnData::GetUpgradeSlotTagForStage(int32 StageIndex) const
{
	int32 Index = StageIndex - 1;
	if (Index < 0 || Index >= UpgradeSlotOrder.Num())
	{
		Index = 0;
	}
	
	if (UpgradeSlotOrder.IsValidIndex(Index))
	{
		return UpgradeSlotOrder[Index];
	}
	return FGameplayTag();
}
