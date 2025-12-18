// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SFDrawShapeLibrary.generated.h"

/**
 * 
 */
UCLASS()
class SF_API USFDrawShapeLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	// 타원 디버그 드로우
	UFUNCTION(BlueprintCallable, Category="SF|Debug", meta=(WorldContext="WorldContext", DevelopmentOnly))
	static void DrawDebugEllipse(const UObject* WorldContext, const FVector& Center, const FVector& ForwardDir, const FVector& RightDir, float ForwardRadius, float SideRadius, float HalfHeight = 0.f, FColor Color = FColor::Green, float Duration = 2.0f, float Thickness = 2.0f, int32 Segments = 32);
};
