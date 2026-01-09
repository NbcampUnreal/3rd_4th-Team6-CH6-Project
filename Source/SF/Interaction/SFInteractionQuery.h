#pragma once

#include "SFInteractionQuery.generated.h"

USTRUCT(BlueprintType)
struct FSFInteractionQuery
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AActor> RequestingAvatar;
	
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AController> RequestingController;
	
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<UObject> OptionalObjectData;
};
