// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Enemy/SFEnemyAnimInstance.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Character/SFCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SFEnemyAnimInstance)

USFEnemyAnimInstance::USFEnemyAnimInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, DisplacementSinceLastUpdate(0.0f)
	, DisplacementSpeed(0.0f)
	, WorldLocation(FVector::ZeroVector)
	, WorldRotation(FRotator::ZeroRotator)
	, bHasVelocity(false)
	, WorldVelocity(FVector::ZeroVector)
	, WorldVelocity2D(FVector::ZeroVector)
	, LocalVelocity2D(FVector::ZeroVector)
	, bHasAcceleration(false)
	, WorldAcceleration2D(FVector::ZeroVector)
	, LocalAcceleration2D(FVector::ZeroVector)
	, bIsFirstUpdate(true)
{
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
		Character = Cast<ASFCharacterBase>(OwningActor);
		if (Character)
		{
			CachedMovementComponent = Character->GetCharacterMovement();
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
		Character = Cast<ASFCharacterBase>(GetOwningActor());
		if (Character && !CachedMovementComponent)
		{
			CachedMovementComponent = Character->GetCharacterMovement();
		}
	}

	if (Character && CachedMovementComponent)
	{
		if (!bIsFirstUpdate)
		{
			PreviousWorldLocation = CachedLocation;
		}
		else
		{
			PreviousWorldLocation = Character->GetActorLocation();
			bIsFirstUpdate = false;
		}

		// Location & Rotation 캐싱
		CachedLocation = Character->GetActorLocation();
		CachedRotation = Character->GetActorRotation();

		// Velocity 캐싱
		WorldVelocity = Character->GetVelocity();
		WorldVelocity2D = FVector(WorldVelocity.X, WorldVelocity.Y, 0.0f);

		// Acceleration 캐싱
		FVector Acceleration = CachedMovementComponent->GetCurrentAcceleration();
		WorldAcceleration2D = FVector(Acceleration.X, Acceleration.Y, 0.0f);
	}
}

void USFEnemyAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	if (!Character)
	{
		return;
	}

	UpdateLocationData(DeltaSeconds);
	UpdateRotationData();
	UpdateVelocityData();
	UpdateAccelerationData();
}

UCharacterMovementComponent* USFEnemyAnimInstance::GetMovementComponent()
{
	AActor* OwningActor = GetOwningActor();
	if (OwningActor)
	{
		Character = Cast<ACharacter>(OwningActor);
		if (Character)
		{
			return Character->GetCharacterMovement();
		}
	}
	return nullptr;
}

#pragma endregion 

#pragma region ThreadSafeAnimationFunction
//위치값 계산  -> Distance Matching을 위해서 변수 계산
void USFEnemyAnimInstance::UpdateLocationData(float DeltaSeconds)
{
	if (!Character)
	{
		return;
	}
	WorldLocation = CachedLocation;

	// Displacement 계산
	DisplacementSinceLastUpdate = FVector::Dist(WorldLocation, PreviousWorldLocation);

	// Displacement Speed 계산
	if (DeltaSeconds > 0.0f)
	{
		DisplacementSpeed = DisplacementSinceLastUpdate / DeltaSeconds;
	}
	else
	{
		DisplacementSpeed = 0.0f;
	}
	
}
//회전값 계산
void USFEnemyAnimInstance::UpdateRotationData()
{
    if (!Character)
    {
        return;
    }
    WorldRotation = CachedRotation;
}
//이동 속도 계산
void USFEnemyAnimInstance::UpdateVelocityData()
{
	if (!Character)
	{
		return;
	}
	LocalVelocity2D = CachedRotation.UnrotateVector(WorldVelocity2D);
	const float VelocityLengthSquared = LocalVelocity2D.SizeSquared2D();
	bHasVelocity = !FMath::IsNearlyZero(VelocityLengthSquared);
}
// 가속도 계산
void USFEnemyAnimInstance::UpdateAccelerationData()
{
	if (!Character)
	{
		return;
	}
	
	LocalAcceleration2D = CachedRotation.UnrotateVector(WorldAcceleration2D);
	const float AccelerationLengthSquared = LocalAcceleration2D.SizeSquared2D();
	bHasAcceleration = !FMath::IsNearlyZero(AccelerationLengthSquared);
}
#pragma endregion