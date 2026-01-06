// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SFDamageWidget.generated.h"

class UTextBlock;
class UWidgetAnimation;

/**
 * 데미지 텍스트 전용 위젯 클래스
 */
UCLASS()
class SF_API USFDamageWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 외부에서 데미지 값을 세팅하고 애니메이션을 시작하는 함수
	void SetupDamageText(float DamageAmount, const FVector& WorldLocation);

protected:
	virtual void NativeConstruct() override;

	// 위젯 애니메이션이 끝나면 자동으로 호출될 함수
	virtual void OnAnimationFinished_Implementation(const UWidgetAnimation* Animation) override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_DamageText;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> Anim_PopUp;
};
