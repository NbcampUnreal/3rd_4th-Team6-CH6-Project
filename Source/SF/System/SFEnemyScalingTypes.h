#pragma once

#include "CoreMinimal.h"
#include "SFEnemyScalingTypes.generated.h"

/**
 * Enemy 스케일링에 필요한 컨텍스트 데이터
 */
USTRUCT(BlueprintType)
struct SF_API FSFEnemyScalingContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 StageIndex = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 SubStageIndex = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 PlayerCount = 1;

	UPROPERTY(BlueprintReadOnly)
	bool bIsBossStage = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsFinalStage = false;

	bool IsValid() const { return PlayerCount > 0; }
};