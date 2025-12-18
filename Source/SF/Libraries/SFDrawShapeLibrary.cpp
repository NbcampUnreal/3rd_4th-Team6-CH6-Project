#include "SFDrawShapeLibrary.h"

void USFDrawShapeLibrary::DrawDebugEllipse(const UObject* WorldContext, const FVector& Center, const FVector& ForwardDir, const FVector& RightDir, float ForwardRadius, float SideRadius, float HalfHeight, FColor Color, float Duration, float Thickness, int32 Segments)
{
#if ENABLE_DRAW_DEBUG
	if (!WorldContext)
	{
		return;
	}

	UWorld* World = WorldContext->GetWorld();
	if (!World)
	{
		return;
	}

	if (Segments < 3)
	{
		Segments = 3;
	}

	const float AngleStep = 2.f * PI / Segments;

	// 바닥 타원
	FVector BottomCenter = Center;
	if (HalfHeight > 0.f)
	{
		BottomCenter.Z -= HalfHeight;
	}

	FVector PrevBottomPoint = BottomCenter + ForwardDir * ForwardRadius;
	FVector PrevTopPoint = PrevBottomPoint + FVector(0.f, 0.f, HalfHeight * 2.f);

	for (int32 i = 1; i <= Segments; ++i)
	{
		float Angle = AngleStep * i;
		float X = FMath::Cos(Angle) * ForwardRadius;
		float Y = FMath::Sin(Angle) * SideRadius;

		FVector CurrentBottomPoint = BottomCenter + ForwardDir * X + RightDir * Y;

		// 바닥 타원
		DrawDebugLine(World, PrevBottomPoint, CurrentBottomPoint, Color, false, Duration, 0, Thickness);

		// 높이가 있으면 상단 타원 + 수직선
		if (HalfHeight > 0.f)
		{
			FVector CurrentTopPoint = CurrentBottomPoint + FVector(0.f, 0.f, HalfHeight * 2.f);

			// 상단 타원
			DrawDebugLine(World, PrevTopPoint, CurrentTopPoint, Color, false, Duration, 0, Thickness);

			// 수직선 (4개만)
			if (i % (Segments / 4) == 0)
			{
				DrawDebugLine(World, CurrentBottomPoint, CurrentTopPoint, Color, false, Duration, 0, Thickness);
			}

			PrevTopPoint = CurrentTopPoint;
		}

		PrevBottomPoint = CurrentBottomPoint;
	}
#endif
}