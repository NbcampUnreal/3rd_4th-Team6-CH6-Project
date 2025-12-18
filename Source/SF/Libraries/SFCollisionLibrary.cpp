// Fill out your copyright notice in the Description page of Project Settings.


#include "SFCollisionLibrary.h"

#include "Kismet/KismetSystemLibrary.h"

bool USFCollisionLibrary::EllipseOverlapActors(const UObject* WorldContext, const FVector& Center,const FVector& ForwardDir, float ForwardRadius, float SideRadius, float HalfHeight, const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes, TSubclassOf<AActor> ActorClassFilter, const TArray<AActor*>& ActorsToIgnore, TArray<AActor*>& OutActors)
{
	OutActors.Reset();

	if (!WorldContext || ForwardRadius <= 0.f || SideRadius <= 0.f)
	{
		return false;
	}

	// 방향 벡터 정규화 및 RightDir 계산
	FVector Forward = ForwardDir.GetSafeNormal2D();
	if (Forward.IsNearlyZero())
	{
		Forward = FVector::ForwardVector;
	}
	FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();

	// 1차 필터링: 타원을 감싸는 원으로 SphereOverlap
	float MaxRadius = FMath::Max(ForwardRadius, SideRadius);
	TArray<AActor*> SphereOverlappedActors;

	bool bHasOverlap = UKismetSystemLibrary::SphereOverlapActors(
		WorldContext,
		Center,
		MaxRadius,
		ObjectTypes,
		ActorClassFilter,
		ActorsToIgnore,
		SphereOverlappedActors
	);

	if (!bHasOverlap)
	{
		return false;
	}

	// 2차 필터링: 타원 내부 판정
	for (AActor* Actor : SphereOverlappedActors)
	{
		if (!Actor)
		{
			continue;
		}

		FVector ToTarget = Actor->GetActorLocation() - Center;

		// 높이 체크 (HalfHeight > 0 일 때만)
		if (HalfHeight > 0.f)
		{
			if (FMath::Abs(ToTarget.Z) > HalfHeight)
			{
				continue;
			}
		}

		// 타원 판정 (XY 평면)
		ToTarget.Z = 0.f;

		float ForwardDist = FVector::DotProduct(ToTarget, Forward);
		float RightDist = FVector::DotProduct(ToTarget, Right);

		float NormalizedForward = ForwardDist / ForwardRadius;
		float NormalizedRight = RightDist / SideRadius;
		float EllipseValue = (NormalizedForward * NormalizedForward) + (NormalizedRight * NormalizedRight);

		if (EllipseValue <= 1.0f)
		{
			OutActors.Add(Actor);
		}
	}

	return OutActors.Num() > 0;
}
