// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Animation/AnimInstance.h"
#include "SFEnemyAnimInstance.generated.h"

// [전방 선언] 헤더 의존성을 줄이기 위해 클래스 미리 선언
class ASFCharacterBase;
class UCharacterMovementComponent;
class UAbilitySystemComponent;
class UCharacterTrajectoryComponent; // [모션 매칭] 컴포넌트 클래스 전방 선언

UENUM(BlueprintType)
enum class AE_CardinalDirection : uint8
{
    Forward    UMETA(DisplayName = "Forward"),
    Right      UMETA(DisplayName = "Right"),
    Backward   UMETA(DisplayName = "Backward"),
    Left       UMETA(DisplayName = "Left")
};

UENUM(BlueprintType)
enum class ERootYawOffsetMode : uint8
{
    Accumulate,  // 누적
    Hold,        // 유지
    BlendOut     // 감쇠
};

UCLASS(Abstract)
class SF_API USFEnemyAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    USFEnemyAnimInstance(const FObjectInitializer& ObjectInitializer);

    virtual void InitializeWithAbilitySystem(UAbilitySystemComponent* ASC);

    // =========================================================================
    // [모션 매칭 섹션] 추가된 변수들
    // =========================================================================
    
    // 1. 모션 매칭 사용 여부 스위치 (보스 BP에서 True로 설정)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bUseMotionMatching = false;

    // 2. 궤적 컴포넌트 접근자
    // (데이터를 직접 복사하지 않고, 이 포인터를 AnimGraph의 Pose History 노드에 연결합니다)
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    TObjectPtr<UCharacterTrajectoryComponent> TrajectoryComponent;

    // =========================================================================

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;
    virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

    // 기존 함수들
    void ApplySpringToRootYawOffset(float DeltaSeconds);

    UFUNCTION(BlueprintPure, Category = "Animation")
    UCharacterMovementComponent* GetMovementComponent();
    
    // Thread Safe Update Functions
    UFUNCTION(BlueprintCallable, Category = "Thread Safe Function", meta = (BlueprintThreadSafe))
    void UpdateLocationData(float DeltaSeconds);

    UFUNCTION(BlueprintCallable, Category = "Thread Safe Function", meta = (BlueprintThreadSafe))
    void UpdateRotationData();

    UFUNCTION(BlueprintCallable, Category = "Thread Safe Function", meta = (BlueprintThreadSafe))
    void UpdateVelocityData();

    UFUNCTION(BlueprintCallable, Category = "Thread Safe Function", meta = (BlueprintThreadSafe))
    void UpdateAccelerationData();

    UFUNCTION(BlueprintCallable, Category = "Thread Safe Function", meta = (BlueprintThreadSafe))
    bool ShouldDistanceMatchStop() const;

    UFUNCTION(BlueprintCallable, Category = "Thread Safe Function", meta = (BlueprintThreadSafe))
    float GetPredictedStopDistance() const;

    UFUNCTION(BlueprintCallable, Category = "Thread Safe Function", meta = (BlueprintThreadSafe))
    AE_CardinalDirection GetCardinalDirectionFromAngle(float Angle, float DeadZone, AE_CardinalDirection CurrentDirection, bool bUseCurrentDirection) const;

    // Turn In Place
    void UpdateTurnInPlace(float DeltaSeconds);
    
    // RootYawOffset 모드별 처리
    void ProcessAccumulateMode(float DeltaYaw);
    void ProcessHoldMode();
    void ProcessBlendOutMode(float DeltaSeconds);

    // 각도 정규화
    float NormalizeAxis(float Angle);

    // Spring 시뮬레이션
    float SpringInterpolate(float Current, float Target, float DeltaTime, float& Velocity, float Stiffness, float DampingRatio, float Mass);

public:
    UFUNCTION(BlueprintCallable, Category = "Turn In Place")
    void ProcessRemainingTurnYaw(float RemainingTurnYaw);

    UFUNCTION(BlueprintCallable, Category = "Turn In Place")
    void OnTurnInPlaceCompleted();

protected:
    UPROPERTY(EditDefaultsOnly, Category = "GameplayTags")
    FGameplayTagBlueprintPropertyMap GameplayTagPropertyMap;

    UPROPERTY(Transient, BlueprintReadOnly, Category = "Character")
    TObjectPtr<ACharacter> Character;

    UPROPERTY(Transient, BlueprintReadOnly, Category = "Character")
    TObjectPtr<UCharacterMovementComponent> CachedMovementComponent;

private:
    FVector CachedLocation;
    FVector PreviousWorldLocation;
    bool bIsFirstUpdate;
    FRotator CachedRotation;
    FVector CachedWorldVelocity;
    FVector CachedWorldVelocity2D;
    FVector CachedWorldAcceleration2D;
    FVector PreviousWorldVelocity2D;
    FRotator PreviousRotation;

protected:
    // Location Data
    UPROPERTY(BlueprintReadOnly, Category = "Location Data")
    FVector WorldLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Location Data")
    float DisplacementSinceLastUpdate;

    UPROPERTY(BlueprintReadOnly, Category = "Location Data")
    float DisplacementSpeed;

    // Rotation Data 
    UPROPERTY(BlueprintReadOnly, Category = "Rotation Data")
    FRotator WorldRotation;

    // Velocity Data 
    UPROPERTY(BlueprintReadOnly, Category = "Velocity Data")
    bool bHasVelocity;

    UPROPERTY(BlueprintReadOnly, Category = "Velocity Data")
    FVector WorldVelocity2D;

    UPROPERTY(BlueprintReadOnly, Category = "Velocity Data")
    FVector LocalVelocity2D;
    
    UPROPERTY(BlueprintReadOnly, Category = "Velocity Data")
    float LocalVelocityDirectionAngle;

    UPROPERTY(BlueprintReadOnly, Category = "Velocity Data")
    AE_CardinalDirection LocalVelocityDirection;

    UPROPERTY(BlueprintReadOnly, Category = "Velocity Data")
    bool bWasMovingLastFrame;

    UPROPERTY(BlueprintReadOnly, Category = "Cardinal Direction")
    float CardinalDirectionDeadZone;

    // Acceleration Data
    UPROPERTY(BlueprintReadOnly, Category = "Acceleration Data")
    bool bHasAcceleration;

    UPROPERTY(BlueprintReadOnly, Category = "Acceleration Data")
    FVector WorldAcceleration2D;

    UPROPERTY(BlueprintReadOnly, Category = "Acceleration Data")
    FVector LocalAcceleration2D;

    // Turn In Place Data
    UPROPERTY(BlueprintReadWrite, Category = "Turn In Place")
    ERootYawOffsetMode RootYawOffsetMode = ERootYawOffsetMode::Accumulate;

    UPROPERTY(BlueprintReadWrite, Category = "Turn In Place")
    float RootYawOffset = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn In Place")
    float TurnInPlaceThreshold = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn In Place")
    float TurnInPlaceThreshold_180 = 135.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn In Place")
    float BlendOutSpeed = 5.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Turn In Place")
    bool bIsTurningInPlace = false;

    UPROPERTY(BlueprintReadOnly, Category = "Turn In Place")
    float TurnDirection = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Turn In Place")
    float TurnAngle = 90.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Turn In Place")
    float TurnYawCurveValue = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Turn In Place")
    float PreviousTurnYawCurveValue = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Turn In Place")
    float PreviousRemainingTurnYaw;

    UPROPERTY(BlueprintReadWrite, Category = "Turn In Place")
    float YawDeltaSinceLastUpdate = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Turn In Place")
    bool bEnableRootYawOffset = true;

    UPROPERTY(BlueprintReadWrite, Category = "Turn In Place")
    float SmoothedRootYawOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn In Place|Spring")
    float SpringStiffness = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn In Place|Spring")
    float SpringDampingRatio = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn In Place|Spring")
    float SpringMass = 1.0f;

    float SpringVelocity = 0.0f;
    float SpringCurrentValue = 0.0f;
};