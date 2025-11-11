// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Animation/AnimInstance.h"
#include "SFEnemyAnimInstance.generated.h"

class ACharacter;
class UCharacterMovementComponent;
/**
 * 
 */
UCLASS(Abstract)
class SF_API USFEnemyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	USFEnemyAnimInstance(const FObjectInitializer& ObjectInitializer);

	virtual void InitializeWithAbilitySystem(UAbilitySystemComponent* ASC);

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

	UFUNCTION(BlueprintPure)
	UCharacterMovementComponent* GetMovementComponent();
	
	// ========== Thread Safe Update Functions ==========
	UFUNCTION(BlueprintCallable, Category = "Thread Safe Function", meta = (BlueprintThreadSafe))
	void UpdateLocationData(float DeltaSeconds);

	UFUNCTION(BlueprintCallable, Category = "Thread Safe Function", meta = (BlueprintThreadSafe))
	void UpdateRotationData();

	UFUNCTION(BlueprintCallable, Category = "Thread Safe Function", meta = (BlueprintThreadSafe))
	void UpdateVelocityData();

	UFUNCTION(BlueprintCallable, Category = "Thread Safe Function", meta = (BlueprintThreadSafe))
	void UpdateAccelerationData();

	


// Proprties 
protected:
	UPROPERTY(EditDefaultsOnly, Category = "GameplayTags")
	FGameplayTagBlueprintPropertyMap GameplayTagPropertyMap;

	// ========== Location Data ==========
	UPROPERTY(BlueprintReadOnly, Category = "Location Data", meta = (AllowPrivateAccess = "true"))
	float DisplacementSinceLastUpdate;

	UPROPERTY(BlueprintReadOnly, Category = "Location Data", meta = (AllowPrivateAccess = "true"))
	float DisplacementSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Location Data", meta = (AllowPrivateAccess = "true"))
	FVector WorldLocation;

	UPROPERTY(Transient)
	FVector CachedLocation;

	// ========== Rotation Data ==========
	UPROPERTY(BlueprintReadOnly, Category = "Rotation Data", meta = (AllowPrivateAccess = "true"))
	FRotator WorldRotation;
	
	UPROPERTY(Transient)
	FRotator CachedRotation;
	// ========== Velocity Data ==========
	UPROPERTY(BlueprintReadOnly, Category = "Velocity Data", meta = (AllowPrivateAccess = "true"))
	bool bHasVelocity;

	UPROPERTY(BlueprintReadOnly, Category = "Velocity Data", meta = (AllowPrivateAccess = "true"))
	FVector WorldVelocity;

	UPROPERTY(BlueprintReadOnly, Category = "Velocity Data", meta = (AllowPrivateAccess = "true"))
	FVector WorldVelocity2D;

	UPROPERTY(BlueprintReadOnly, Category = "Velocity Data", meta = (AllowPrivateAccess = "true"))
	FVector LocalVelocity2D;

	// ========== Acceleration Data ==========
	UPROPERTY(BlueprintReadOnly, Category = "Acceleration Data", meta = (AllowPrivateAccess = "true"))
	bool bHasAcceleration;

	UPROPERTY(BlueprintReadOnly, Category = "Acceleration Data", meta = (AllowPrivateAccess = "true"))
	FVector WorldAcceleration2D;

	UPROPERTY(BlueprintReadOnly, Category = "Acceleration Data", meta = (AllowPrivateAccess = "true"))
	FVector LocalAcceleration2D;

	UPROPERTY(BlueprintReadOnly, Category = "Acceleration Data", meta = (AllowPrivateAccess = "true"))
	bool bIsFirstUpdate;

	// ========== Character Data ==========
	UPROPERTY(BlueprintReadOnly, Category = "Character Data", meta = (AllowPrivateAccess = "true"))
	ACharacter* Character;

	UPROPERTY(BlueprintReadOnly, Category = "Character Data", meta = (AllowPrivateAccess = "true"))
	FVector PreviousWorldLocation;

	UPROPERTY(BlueprintReadOnly, Category = "Character Data")
	UCharacterMovementComponent* CachedMovementComponent;
	


	

	
	
};
