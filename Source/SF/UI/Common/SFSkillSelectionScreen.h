#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "SFSkillSelectionScreen.generated.h"

class URewardCardBase;
class USFGameplayAbility;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSelectionCompleteSignature);
/**
 * 
 */
UCLASS()
class SF_API USFSkillSelectionScreen : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// 외부에서 스테이지 인덱스 설정 후 초기화
	// StageIndex: 0 = 1스테이지 보스, 1 = 2스테이지 보스, 2 = 3스테이지 보스
	UFUNCTION(BlueprintCallable, Category = "UI|Selection")
	void InitializeSelection(int32 InStageIndex);

protected:
	// 카드 선택 콜백
	UFUNCTION()
	void OnCardSelected(int32 CardIndex, TSubclassOf<USFGameplayAbility> SelectedAbilityClass);

	UFUNCTION(BlueprintCallable, Category = "SF|UI")
	void CloseSelection();

public:
	
	// 선택 완료 델리게이트 (PlayerController에서 바인딩)
	UPROPERTY(BlueprintAssignable, Category = "SF|UI")
	FOnSelectionCompleteSignature OnSelectionCompleteDelegate;

protected:
	// 카드 위젯 바인딩
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<URewardCardBase> RewardCard_01;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<URewardCardBase> RewardCard_02;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<URewardCardBase> RewardCard_03;
	
	// 현재 스테이지 인덱스 (0, 1, 2)
	UPROPERTY(BlueprintReadOnly, Category = "UI|Selection")
	int32 CurrentStageIndex;

	// 현재 업그레이드 중인 슬롯의 InputTag
	UPROPERTY(BlueprintReadOnly, Category = "UI|Selection")
	FGameplayTag CurrentUpgradeSlotTag;
};
