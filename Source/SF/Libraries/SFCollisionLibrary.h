#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SFCollisionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class SF_API USFCollisionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * 타원형 범위 내 액터 검출
	 * 내부적으로 SphereOverlap 후 타원 필터링
	 */
	UFUNCTION(BlueprintCallable, Category="SF|Collision", meta=(WorldContext="WorldContext"))
	static bool EllipseOverlapActors(const UObject* WorldContext, const FVector& Center, const FVector& ForwardDir, float ForwardRadius, float SideRadius, float HalfHeight, const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes, TSubclassOf<AActor> ActorClassFilter, const TArray<AActor*>& ActorsToIgnore, TArray<AActor*>& OutActors);
};
