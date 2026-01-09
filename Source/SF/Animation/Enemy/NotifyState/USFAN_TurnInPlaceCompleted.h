// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "USFAN_TurnInPlaceCompleted.generated.h"

/**
 * 
 */
UCLASS()
class SF_API UUSFAN_TurnInPlaceCompleted : public UAnimNotify
{
	GENERATED_BODY()
public:
	virtual void Notify	(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override
	{
		return TEXT("Turn In Place Completed");
	}
};
#pragma once

