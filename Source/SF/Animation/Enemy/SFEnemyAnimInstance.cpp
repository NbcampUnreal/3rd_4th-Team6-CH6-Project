// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/Enemy/SFEnemyAnimInstance.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Character/SFCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AnimCharacterMovementLibrary.h"

// =========================================================================
// [모션 매칭 섹션] 필수 헤더 추가
// =========================================================================
#include "Character/Enemy/SFEnemy.h"        // 캐릭터 캐스팅용
#include "CharacterTrajectoryComponent.h"   // 컴포넌트 접근용
// =========================================================================

#include UE_INLINE_GENERATED_CPP_BY_NAME(SFEnemyAnimInstance)

USFEnemyAnimInstance::USFEnemyAnimInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, Character(nullptr)
	, CachedMovementComponent(nullptr)
	, CachedLocation(FVector::ZeroVector)
	, PreviousWorldLocation(FVector::ZeroVector)
	, bIsFirstUpdate(true)
	, CachedRotation(FRotator::ZeroRotator)
	, CachedWorldVelocity(FVector::ZeroVector)
	, CachedWorldVelocity2D(FVector::ZeroVector)
	, CachedWorldAcceleration2D(FVector::ZeroVector)
	, WorldLocation(FVector::ZeroVector)
	, DisplacementSinceLastUpdate(0.0f)
	, DisplacementSpeed(0.0f)
	, WorldRotation(FRotator::ZeroRotator)
	, bHasVelocity(false)
	, WorldVelocity2D(FVector::ZeroVector)
	, LocalVelocity2D(FVector::ZeroVector)
	, bHasAcceleration(false)
	, WorldAcceleration2D(FVector::ZeroVector)
	, LocalAcceleration2D(FVector::ZeroVector)
	, LocalVelocityDirectionAngle(0.0f)
	, CardinalDirectionDeadZone(10.f)
	, bWasMovingLastFrame(false)
	, PreviousWorldVelocity2D(FVector::ZeroVector)
	, PreviousRotation(FRotator::ZeroRotator)
	, PreviousRemainingTurnYaw(0.0f)
	, SmoothedRootYawOffset(0.0f)
	, TurnAngle(90.0f)
{
    // [초기화] 모션 매칭 변수
    bUseMotionMatching = false;
    TrajectoryComponent = nullptr;
}

#pragma region AnimationFunction

void USFEnemyAnimInstance::InitializeWithAbilitySystem(UAbilitySystemComponent* ASC)
{
	check(ASC);
	GameplayTagPropertyMap.Initialize(this, ASC);
}

void USFEnemyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	if (AActor* OwningActor = GetOwningActor())
	{
		Character = Cast<ACharacter>(OwningActor);
		if (Character)
		{
			CachedMovementComponent = Character->GetCharacterMovement();
		}
        
        // =====================================================================
        // [모션 매칭] 궤적 컴포넌트 캐싱 (한 번만 찾아서 저장)
        // =====================================================================
        if (auto* EnemyPawn = Cast<ASFEnemy>(OwningActor))
        {
            if (EnemyPawn->CharacterTrajectory)
            {
                TrajectoryComponent = EnemyPawn->CharacterTrajectory;
            }
        }
        
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningActor))
		{
			InitializeWithAbilitySystem(ASC);
		}
	}
}

void USFEnemyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!Character)
	{
		Character = Cast<ACharacter>(GetOwningActor());
		if (Character && !CachedMovementComponent)
		{
			CachedMovementComponent = Character->GetCharacterMovement();
		}
	}

	if (!Character || !CachedMovementComponent)
	{
		return;
	}

	if (!bIsFirstUpdate)
	{
		PreviousWorldLocation = CachedLocation;
		PreviousWorldVelocity2D = CachedWorldVelocity2D;
		PreviousRotation = CachedRotation;
	}
	else
	{
		PreviousWorldLocation = Character->GetActorLocation();
		PreviousWorldVelocity2D = FVector::ZeroVector;
		PreviousRotation = Character->GetActorRotation();
		bIsFirstUpdate = false;
	}

	CachedLocation = Character->GetActorLocation();
	CachedRotation = Character->GetActorRotation();  

	CachedWorldVelocity = CachedMovementComponent->Velocity;
	CachedWorldVelocity2D = FVector(CachedWorldVelocity.X, CachedWorldVelocity.Y, 0.0f);

	const FVector Acceleration = CachedMovementComponent->GetCurrentAcceleration();
	CachedWorldAcceleration2D = FVector(Acceleration.X, Acceleration.Y, 0.0f);


    // =========================================================================
    // [모션 매칭 vs 기존 로직 분기]
    // =========================================================================
    if (bUseMotionMatching)
    {
        // 모션 매칭이 켜져 있으면, 기존 TurnInPlace 로직을 위한 Yaw 계산을 초기화합니다.
        PreviousRemainingTurnYaw = 0.0f;
    }
    else
    {
        // 기존 로직 사용 (일반 몬스터)
        if (bIsTurningInPlace)
        {
            float CurrentRemainingYaw = GetCurveValue(FName("RemainingTurnYaw"));
            float DeltaYaw = PreviousRemainingTurnYaw - CurrentRemainingYaw;
            
            if (FMath::Abs(DeltaYaw) > UE_SMALL_NUMBER)
            {
                ProcessRemainingTurnYaw(DeltaYaw);
            }
            
            PreviousRemainingTurnYaw = CurrentRemainingYaw;
        }
        else
        {
            PreviousRemainingTurnYaw = 0.0f;
        }
    }
}

void USFEnemyAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	UpdateLocationData(DeltaSeconds);
	UpdateRotationData();
	UpdateVelocityData();
	UpdateAccelerationData();
	
    // =========================================================================
    // [모션 매칭 vs 기존 로직 분기]
    // =========================================================================
    if (bUseMotionMatching)
    {
        // 모션 매칭 사용 시에는 기존의 회전 보정 값들을 모두 0으로 만들어 충돌을 방지합니다.
        bIsTurningInPlace = false;
        RootYawOffset = 0.0f;
        SmoothedRootYawOffset = 0.0f;
        SpringCurrentValue = 0.0f;
    }
    else
    {
        // 기존 로직 수행
        UpdateTurnInPlace(DeltaSeconds);
        
        if (bEnableRootYawOffset)
        {
            ApplySpringToRootYawOffset(DeltaSeconds);
        }
    }
}

UCharacterMovementComponent* USFEnemyAnimInstance::GetMovementComponent()
{
	if (CachedMovementComponent)
	{
		return CachedMovementComponent;
	}

	if (AActor* OwningActor = GetOwningActor())
	{
		if (ACharacter* OwningCharacter = Cast<ACharacter>(OwningActor))
		{
			CachedMovementComponent = OwningCharacter->GetCharacterMovement();
			return CachedMovementComponent;
		}
	}
	return nullptr;
}

#pragma endregion 

#pragma region ThreadSafeAnimationFunction

void USFEnemyAnimInstance::UpdateLocationData(float DeltaSeconds)
{
	WorldLocation = CachedLocation;
	DisplacementSinceLastUpdate = FVector::Dist(WorldLocation, PreviousWorldLocation);

	if (DeltaSeconds > 0.0f)
	{
		DisplacementSpeed = DisplacementSinceLastUpdate / DeltaSeconds;
	}
	else
	{
		DisplacementSpeed = 0.0f;
	}
}

void USFEnemyAnimInstance::UpdateRotationData()
{
	WorldRotation = CachedRotation;
}

void USFEnemyAnimInstance::UpdateVelocityData()
{
	WorldVelocity2D = CachedWorldVelocity2D;
	LocalVelocity2D = CachedRotation.UnrotateVector(WorldVelocity2D);
	const float VelocityLength = LocalVelocity2D.Size();
	
	if (VelocityLength > 1.0f)
	{
		bHasVelocity = true;
	}
	else
	{
		bHasVelocity = false;
	}

	LocalVelocityDirectionAngle = FMath::RadiansToDegrees(FMath::Atan2(LocalVelocity2D.Y, LocalVelocity2D.X));
	LocalVelocityDirection = GetCardinalDirectionFromAngle(LocalVelocityDirectionAngle, CardinalDirectionDeadZone, LocalVelocityDirection, bWasMovingLastFrame);
	bWasMovingLastFrame = bHasVelocity;
}

void USFEnemyAnimInstance::UpdateAccelerationData()
{
	if (GetOwningActor() && GetOwningActor()->HasAuthority())
	{
		WorldAcceleration2D = CachedWorldAcceleration2D;
	}
	else
	{
		const UWorld* World = GetWorld();
		if (World)
		{
			const float DeltaTime = World->GetDeltaSeconds();
			if (DeltaTime > UE_SMALL_NUMBER) 
			{
				WorldAcceleration2D = (CachedWorldVelocity2D - PreviousWorldVelocity2D) / DeltaTime;
			}
			else
			{
				WorldAcceleration2D = FVector::ZeroVector;
			}
		}
		else
		{
			WorldAcceleration2D = FVector::ZeroVector;
		}
	}
    
	LocalAcceleration2D = CachedRotation.UnrotateVector(WorldAcceleration2D);
	const float AccelerationLength = LocalAcceleration2D.Size();
	bHasAcceleration = AccelerationLength > 1.0f;
}

bool USFEnemyAnimInstance::ShouldDistanceMatchStop() const
{
	return bHasVelocity && (!bHasAcceleration);
}

float USFEnemyAnimInstance::GetPredictedStopDistance() const
{
	if (!CachedMovementComponent) return 0.f;

	const FVector Velocity = CachedMovementComponent->GetLastUpdateVelocity();
	const bool bUseSeparateBrakingFriction = CachedMovementComponent->bUseSeparateBrakingFriction;
	const float BrakingFriction = CachedMovementComponent->BrakingFriction;
	const float GroundFriction = CachedMovementComponent->GroundFriction;
	const float BrakingFrictionFactor = CachedMovementComponent->BrakingFrictionFactor;
	const float BrakingDecel = CachedMovementComponent->BrakingDecelerationWalking;

	const FVector PredictedStopLoc = UAnimCharacterMovementLibrary::PredictGroundMovementStopLocation(
		Velocity,
		bUseSeparateBrakingFriction,
		BrakingFriction,
		GroundFriction,
		BrakingFrictionFactor,
		BrakingDecel
	);

	return FVector2D(PredictedStopLoc).Length();
}

AE_CardinalDirection USFEnemyAnimInstance::GetCardinalDirectionFromAngle(float Angle, float DeadZone,
	AE_CardinalDirection CurrentDirection, bool bUseCurrentDirection) const
{
	float AbsAngle = FMath::Abs(Angle);
	float FwdDeadZone = DeadZone;
	float BwdDeadZone = DeadZone;

	if (bUseCurrentDirection)
	{
		switch (CurrentDirection)
		{
		case AE_CardinalDirection::Forward:
			FwdDeadZone *= 2.0f;
			break;
		case AE_CardinalDirection::Backward:
			BwdDeadZone *= 2.0f;
			break;
		default:
			break;
		}
	}
	
	if (FwdDeadZone + 45 >= AbsAngle)
	{
		return AE_CardinalDirection::Forward;
	}
	else if (135.0f - BwdDeadZone <= AbsAngle)
	{
		return AE_CardinalDirection::Backward;
	}
	else if (Angle < 0.0f)
	{
		return AE_CardinalDirection::Left;
	}
	else
	{
		return AE_CardinalDirection::Right;
	}
}

#pragma endregion

#pragma region TurnInPlace

void USFEnemyAnimInstance::UpdateTurnInPlace(float DeltaSeconds)
{
	float CurrentYaw = CachedRotation.Yaw;
	float PreviousYaw = PreviousRotation.Yaw;
	float DeltaYaw = FMath::FindDeltaAngleDegrees(PreviousYaw, CurrentYaw);

	if (bHasVelocity)
	{
		if (RootYawOffsetMode != ERootYawOffsetMode::BlendOut)
		{
			RootYawOffsetMode = ERootYawOffsetMode::BlendOut;
			bIsTurningInPlace = false;
		}
		ProcessBlendOutMode(DeltaSeconds);
		return;
	}

	switch (RootYawOffsetMode)
	{
		case ERootYawOffsetMode::Accumulate:
			ProcessAccumulateMode(DeltaYaw);
			break;

		case ERootYawOffsetMode::Hold:
			ProcessHoldMode();
			break;

		case ERootYawOffsetMode::BlendOut:
			ProcessBlendOutMode(DeltaSeconds);
			break;
	}
	if (bIsTurningInPlace)
	{
		
		if (bHasVelocity)
		{
			RootYawOffsetMode = ERootYawOffsetMode::BlendOut;
			bIsTurningInPlace = false;
		}
		
		if (FMath::Abs(RootYawOffset) < 0.1f)
		{
			RootYawOffsetMode = ERootYawOffsetMode::BlendOut;
			bIsTurningInPlace = false;
		}
	}

}

void USFEnemyAnimInstance::ProcessAccumulateMode(float DeltaYaw)
{
	RootYawOffset += (-DeltaYaw);
	RootYawOffset = NormalizeAxis(RootYawOffset);

	float AbsOffset = FMath::Abs(RootYawOffset);

	if (AbsOffset > TurnInPlaceThreshold_180)
	{
		bIsTurningInPlace = true;
		TurnDirection = RootYawOffset > 0.0f ? 1.0f : -1.0f;
		TurnAngle = 180.0f;
		RootYawOffsetMode = ERootYawOffsetMode::Hold;
	}
	else if (AbsOffset > TurnInPlaceThreshold)
	{
		bIsTurningInPlace = true;
		TurnDirection = RootYawOffset > 0.0f ? 1.0f : -1.0f;
		TurnAngle = 90.0f;
		RootYawOffsetMode = ERootYawOffsetMode::Hold;
	}
}

void USFEnemyAnimInstance::ProcessHoldMode()
{
}

void USFEnemyAnimInstance::ProcessBlendOutMode(float DeltaSeconds)
{
	RootYawOffset = FMath::FInterpTo(RootYawOffset, 0.0f, DeltaSeconds, BlendOutSpeed);

	if (FMath::Abs(RootYawOffset) < 0.1f)
	{
		RootYawOffset = 0.0f;
		bIsTurningInPlace = false;
		RootYawOffsetMode = ERootYawOffsetMode::Accumulate;
	}
}

void USFEnemyAnimInstance::ProcessRemainingTurnYaw(float DeltaTurnYaw)
{
	RootYawOffset -= DeltaTurnYaw;
	RootYawOffset = NormalizeAxis(RootYawOffset);
}

void USFEnemyAnimInstance::OnTurnInPlaceCompleted()
{
	RootYawOffsetMode = ERootYawOffsetMode::BlendOut;
	bIsTurningInPlace = false;
	TurnAngle = 90.0f; 
}

float USFEnemyAnimInstance::NormalizeAxis(float Angle)
{
	Angle = FMath::Fmod(Angle, 360.0f);

	if (Angle > 180.0f)
	{
		Angle -= 360.0f;
	}
	else if (Angle < -180.0f)
	{
		Angle += 360.0f;
	}

	return Angle;
}

#pragma endregion

#pragma region Spring (Optional)

void USFEnemyAnimInstance::ApplySpringToRootYawOffset(float DeltaSeconds)
{
	float SpringTarget = RootYawOffset;

	SpringCurrentValue = SpringInterpolate(
		SpringCurrentValue,
		SpringTarget,
		DeltaSeconds,
		SpringVelocity,
		SpringStiffness,
		SpringDampingRatio,
		SpringMass
	);

	SmoothedRootYawOffset = SpringCurrentValue;
	SmoothedRootYawOffset = NormalizeAxis(SmoothedRootYawOffset);
}

float USFEnemyAnimInstance::SpringInterpolate(float Current, float Target, float DeltaTime,
                                               float& Velocity, float Stiffness,
                                               float DampingRatio, float Mass)
{
	if (DeltaTime <= 0.0f || Mass <= 0.0f)
	{
		return Current;
	}

	float Error = Target - Current;
	float SpringForce = Stiffness * Error;
	float DampingForce = 2.0f * DampingRatio * FMath::Sqrt(Stiffness * Mass) * Velocity;
	float TotalForce = SpringForce - DampingForce;
	float Acceleration = TotalForce / Mass;
	Velocity += Acceleration * DeltaTime;
	float NewValue = Current + Velocity * DeltaTime;

	return NewValue;
}

#pragma endregion