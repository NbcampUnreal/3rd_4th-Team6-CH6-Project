// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Common/RewardCardBase.h"

#include "AbilitySystem/Abilities/SFGameplayAbility.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/Button.h"


void URewardCardBase::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Click)
	{
		Btn_Click->OnClicked.AddDynamic(this, &URewardCardBase::OnCardClicked); 
	}
	
}

// TODO : 삭제 예정, 카드 데이터 연동 함수
void URewardCardBase::SetCardData(const FTempCardInfo& InData)
{
	if (Text_Title) Text_Title->SetText(InData.CardName);
	if (Text_Desc) Text_Desc->SetText(InData.Description);

	if (Image_Icon && InData.Icon)
	{
		Image_Icon->SetBrushFromTexture(InData.Icon);
	}

	if (Border_Frame && RarityColors.Contains(InData.Rarity))
	{
		Border_Frame->SetBrushColor(RarityColors[InData.Rarity]);
	}
}

void URewardCardBase::SetCardDataFromAbility(TSubclassOf<USFGameplayAbility> InAbilityClass, int32 InCardIndex)
{
	CurrentCardIndex = InCardIndex;
	CachedAbilityClass = InAbilityClass;

	if (!InAbilityClass)
	{
		return;
	}

	// CDO에서 정보 읽기
	const USFGameplayAbility* AbilityCDO = InAbilityClass->GetDefaultObject<USFGameplayAbility>();
	if (!AbilityCDO)
	{
		return;
	}

	if (Text_Title)
	{
		Text_Title->SetText(AbilityCDO->Name);
	}

	if (Text_Desc)
	{
		Text_Desc->SetText(AbilityCDO->Description);
	}

	if (Image_Icon && AbilityCDO->Icon)
	{
		Image_Icon->SetBrushFromTexture(AbilityCDO->Icon);
	}
}

void URewardCardBase::OnCardClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("%d 번 카드 선택!"), CurrentCardIndex);

	// 선택된 카드 정보 전달
	OnCardSelectedDelegate.Broadcast(CurrentCardIndex, CachedAbilityClass);
}
