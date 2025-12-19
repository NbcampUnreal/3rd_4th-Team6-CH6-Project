#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SFBasicAttackTypes.generated.h"

USTRUCT(BlueprintType)
struct FSFBasicAttackStep
{
	GENERATED_BODY()

	/** 실행할 공격 몽타주 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimMontage> Montage;

	/** 이 단계의 기본 대미지 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageMultiplier = 1.0f;

	/** 차징 공격 단계 여부 (소서러용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsChargeStep = false;

	/** 차징 완료로 간주되는 최소 시간 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bIsChargeStep"))
	float MinChargeTime = 0.5f;

	/** 공격 단계 진행 시 캐릭터에게 일시적으로 부여할 태그 (예: 슈퍼아머) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer TempAbilityTags;
};