// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/SFAbilitySystemComponent.h"
#include "Character/Enemy/SFEnemy.h"
#include "Components/WidgetComponent.h"
#include "UI/Controller/SFWidgetController.h"
#include "SFEnemyWidgetComponent.generated.h"


class UUSFEnemyWidget;
class UAbilitySystemComponent;
class USFCombatSet_Enemy;
class USFPrimarySet_Enemy;
class UCommonBarBase;


UCLASS()
class SF_API USFEnemyWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	USFEnemyWidgetComponent();

	virtual void BeginPlay() override;
	void TryInitializeWidget();

	void OnHealthChanged(const FOnAttributeChangeData& OnAttributeChangeData);
	// 초기화
	void InitializeWidget();
	
	void MarkAsAttackedByLocalPlayer();

protected:
	virtual void InitWidget() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:

	void FaceToCamera();
	void UpdateVisibility(float DeltaTime);
	
	void ShowHealthBar();

	//교전 타이머 만료 시 호출 
	void OnEngagementExpired();

	//교전 타이머 리셋 
	void ResetEngagementTimer();

	// 체력 퍼센트 업데이트
	void UpdateHealthPercent();

private:

	// Widget
	UPROPERTY()
	TObjectPtr<UCommonBarBase> EnemyWidget;

	// 표시 관련
	UPROPERTY(EditAnywhere, Category = "SF|Widget")
	float VisibleDurationAfterHit = 3.f;

	UPROPERTY(EditAnywhere, Category = "SF|Widget")
	float MaxRenderDistance = 3000.f;

	UPROPERTY(EditAnywhere, Category = "SF|Widget")
	float HideOnAngle = 90.f;

	float CurrentVisibleTime = 0.f;

	// 이건 클라이언트 표시 
	bool bEngagedByLocalPlayer = false;

	// 타이머 
	FTimerHandle EngagementTimerHandle;

	//
	float CachedMaxHealth = 0.f;
};