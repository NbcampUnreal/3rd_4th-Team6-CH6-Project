// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGame/SFDamageWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"
#include "Animation/WidgetAnimation.h"


void USFDamageWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void USFDamageWidget::SetupDamageText(float DamageAmount, const FVector& WorldLocation)
{
	// 1. 텍스트 설정
	if (Txt_DamageText)
	{
		Txt_DamageText->SetText(FText::AsNumber(static_cast<int32>(DamageAmount)));
	}

	// 2. 화면 위치 설정 (초기 위치)
	if (APlayerController* PC = GetOwningPlayer())
	{
		FVector2D ScreenPos;
		if (PC->ProjectWorldLocationToScreen(WorldLocation, ScreenPos))
		{
			SetPositionInViewport(ScreenPos);
		}
	}

	// 3. 애니메이션 재생
	if (Anim_PopUp)
	{
		PlayAnimation(Anim_PopUp);
	}
	else
	{
		// 애니메이션이 없으면 바로 삭제 (안전장치)
		RemoveFromParent();
	}
	
}

void USFDamageWidget::OnAnimationFinished_Implementation(const UWidgetAnimation* Animation)
{
	Super::OnAnimationFinished_Implementation(Animation);
		
	// 컨트롤러가 얘를 관리할 필요가 없어짐 -> 참조 오류 원천 차단
	if (Animation == Anim_PopUp)
	{
		RemoveFromParent();
	}
}