// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "USFGC_PlayCosmetic.generated.h"

/**
 * 
 */
UCLASS()
class SF_API UUSFGC_PlayCosmetic : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	virtual void HandleGameplayCue(AActor* Target,
								   EGameplayCueEvent::Type EventType,
								   const FGameplayCueParameters& Parameters) override;
};
