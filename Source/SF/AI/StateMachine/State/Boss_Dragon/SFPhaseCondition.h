// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/Enemy/SFPrimarySet_Enemy.h"
#include "SFPhaseCondition.generated.h"

UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class USFPhaseCondition : public UObject
{
	GENERATED_BODY()

public:
	
	virtual bool IsMet(UAbilitySystemComponent* ASC, AActor* Owner) const { return false; }
};


UCLASS()
class USFHealthPhaseCondition : public USFPhaseCondition
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	float HealthRatio = 0.5f;

	virtual bool IsMet(UAbilitySystemComponent* ASC, AActor* Owner) const override
	{
		if (!ASC) return false;
		float HP = ASC->GetNumericAttribute(USFPrimarySet_Enemy::GetHealthAttribute());
		float MaxHP = ASC->GetNumericAttribute(USFPrimarySet_Enemy::GetMaxHealthAttribute());
		return (HP / MaxHP) <= HealthRatio;
	}
};


USTRUCT(BlueprintType)
struct FSFPhaseData
{
    GENERATED_BODY()
	
    UPROPERTY(EditAnywhere, Instanced, Category = "Conditions")
    TArray<TObjectPtr<USFPhaseCondition>> Conditions;

    UPROPERTY(EditAnywhere, Category = "Phase")
    FGameplayTag TargetPhaseTag;

	bool operator==(const FSFPhaseData& Other) const
	{
		return TargetPhaseTag == Other.TargetPhaseTag;
	}
	
};
