#include "SFSkillSelectionScreen.h"

#include "RewardCardBase.h"
#include "AbilitySystem/Abilities/SFGameplayAbility.h"
#include "Character/SFPawnData.h"
#include "Player/SFPlayerState.h"

void USFSkillSelectionScreen::NativeConstruct()
{
    Super::NativeConstruct();

    if (RewardCard_01)
    {
        RewardCard_01->OnCardSelectedDelegate.AddDynamic(this, &ThisClass::OnCardSelected);
    }
    if (RewardCard_02)
    {
        RewardCard_02->OnCardSelectedDelegate.AddDynamic(this, &ThisClass::OnCardSelected);
    }
    if (RewardCard_03)
    {
        RewardCard_03->OnCardSelectedDelegate.AddDynamic(this, &ThisClass::OnCardSelected);
    }
}

void USFSkillSelectionScreen::NativeDestruct()
{
    if (RewardCard_01)
    {
        RewardCard_01->OnCardSelectedDelegate.RemoveAll(this);
    }
    if (RewardCard_02)
    {
        RewardCard_02->OnCardSelectedDelegate.RemoveAll(this);
    }
    if (RewardCard_03)
    {
        RewardCard_03->OnCardSelectedDelegate.RemoveAll(this);
    }

    Super::NativeDestruct();
}

void USFSkillSelectionScreen::InitializeSelection(int32 InStageIndex)
{
    CurrentStageIndex = InStageIndex;

    // PlayerState에서 PawnData 가져오기
    APlayerController* PC = GetOwningPlayer();
    if (!PC)
    {
        return;
    }

    ASFPlayerState* SFPS = PC->GetPlayerState<ASFPlayerState>();
    if (!SFPS)
    {
        return;
    }

    const USFPawnData* PawnData = SFPS->GetPawnData<USFPawnData>();
    if (!PawnData)
    {
        UE_LOG(LogTemp, Error, TEXT("SelectionScreen: Failed to get PawnData"));
        return;
    }

    // 현재 스테이지에 해당하는 슬롯 태그 가져오기
    CurrentUpgradeSlotTag = PawnData->GetUpgradeSlotTagForStage(CurrentStageIndex);
    if (!CurrentUpgradeSlotTag.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("SelectionScreen: Invalid slot tag for stage %d"), CurrentStageIndex);
        return;
    }

    // 해당 슬롯의 업그레이드 선택지 가져오기
    TArray<TSubclassOf<USFGameplayAbility>> UpgradeOptions = PawnData->GetUpgradeOptionsForSlot(CurrentUpgradeSlotTag);

    // 카드에 데이터 설정
    TArray<URewardCardBase*> Cards = { RewardCard_01, RewardCard_02, RewardCard_03 };
    
    for (int32 i = 0; i < Cards.Num(); ++i)
    {
        if (Cards[i] && UpgradeOptions.IsValidIndex(i))
        {
            Cards[i]->SetCardDataFromAbility(UpgradeOptions[i], i);
        }
    }
}

void USFSkillSelectionScreen::OnCardSelected(int32 CardIndex, TSubclassOf<USFGameplayAbility> SelectedAbilityClass)
{
    if (SelectedAbilityClass && CurrentUpgradeSlotTag.IsValid())
    {
        // PlayerState의 스킬 교체 RPC 호출
        if (ASFPlayerState* PS = GetOwningPlayerState<ASFPlayerState>())
        {
            PS->Server_RequestSkillUpgrade(SelectedAbilityClass, CurrentUpgradeSlotTag);
        }
        
    }

    TArray<URewardCardBase*> Cards = { RewardCard_01, RewardCard_02, RewardCard_03 };
    for (int32 i = 0; i < Cards.Num(); ++i)
    {
        if (Cards[i])
        {
            Cards[i]->SetButtonEnabled(false);
            
            if (i != CardIndex)
            {
                Cards[i]->SetVisibility(ESlateVisibility::Collapsed);
            }
            else
            {
                // 선택된 카드의 애니메이션 완료 바인딩
                Cards[i]->OnSelectAnimationFinished.AddDynamic(this, &ThisClass::CloseSelection);
            }
        }
    }

    if (APlayerController* PC = GetOwningPlayer())
    {
        PC->SetInputMode(FInputModeGameOnly());
        PC->bShowMouseCursor = false;
    }

}

void USFSkillSelectionScreen::CloseSelection()
{
    OnSelectionCompleteDelegate.Broadcast();
}


