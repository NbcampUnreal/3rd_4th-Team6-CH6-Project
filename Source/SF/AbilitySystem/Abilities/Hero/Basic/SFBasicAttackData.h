#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SFBasicAttackTypes.h"
#include "SFBasicAttackData.generated.h"

/**
 * 직업별 기본 공격 시퀀스를 정의하는 데이터 에셋
 */
UCLASS(BlueprintType)
class SF_API USFBasicAttackData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 팔라딘 콤보나 소서러 차징 단계들만 관리 (불필요한 만료 시간 제거)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	TArray<FSFBasicAttackStep> AttackSteps;
};