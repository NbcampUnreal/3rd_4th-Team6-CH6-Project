// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RewardCardBase.generated.h"

class USFGameplayAbility;
class UTextBlock;
class UImage;
class UBorder;
class UButton;

// 리워드 카드 선택시 선택된 어빌리티 정보 전달 델리게이트 
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRewardCardSelectedSignature, int32, CardIndex, TSubclassOf<USFGameplayAbility>, SelectedAbilityClass);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSelectAnimationFinished);

// =========================================================
// [임시 데이터 영역]

// 1. 임시 등급
UENUM(BlueprintType)
enum class ETempCardRarity : uint8
{
	Common,
	Uncommon,
	Rare,
	Epic
};

// =========================================================

UCLASS()
class SF_API URewardCardBase : public UUserWidget
{
	GENERATED_BODY()

// UI Components Binding
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Title;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Desc;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>	Image_Icon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>	Image_Back;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder>	Border_Frame;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Click;
	

	// Design Settings
	// BP 디테일 패널에서 색상 지정 필요 (Enum 색상과 매치)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "UI|Common");
	TMap<ETempCardRarity, FLinearColor> RarityColors;

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UFUNCTION(BlueprintCallable)
	void SetButtonEnabled(bool bEnabled);

	// 어빌리티 클래스로 카드 데이터 설정
	UFUNCTION(BlueprintCallable, Category = "UI|Function")
	void SetCardDataFromAbility(TSubclassOf<USFGameplayAbility> InAbilityClass, int32 InCardIndex);

	// 카드 그래픽 애니메이션 함수 -> BP상에서 구현
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "UI|Animation")
	void PlayCardReveal();

public:

	UFUNCTION(BlueprintCallable, Category = "SF|UI")
	void NotifyAnimationComplete();
	
	// 카드 정보 전달 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "UI|Event")
	FOnRewardCardSelectedSignature OnCardSelectedDelegate;

	UPROPERTY(BlueprintAssignable)
	FOnSelectAnimationFinished OnSelectAnimationFinished;

protected:
	
	// 캐싱된 어빌리티 클래스 (선택 시 전달용)
	UPROPERTY(BlueprintReadOnly, Category = "UI|Data")
	TSubclassOf<USFGameplayAbility> CachedAbilityClass;

	// 몇 번째 카드인지 저장하는 변수
	UPROPERTY(BlueprintReadWrite, Category = "UI|Data")
	int32 CurrentCardIndex;

	FTimerHandle ButtonEnableTimerHandle;
	
private:

	// 카드 선택시 호출 함수
	UFUNCTION()
	void OnCardClicked();
	
};
