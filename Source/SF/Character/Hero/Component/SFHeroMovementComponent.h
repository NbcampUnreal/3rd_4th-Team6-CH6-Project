#pragma once

#include "CoreMinimal.h"
#include "SFCharacterMovementReplication.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SFHeroMovementComponent.generated.h"

class UMotionWarpingComponent;

UENUM(BlueprintType)
enum class ESFSlidingMode : uint8
{
	None,
	
	// 기본 슬라이딩 동작
	Normal,
	
	// 슬라이딩 없이 정지
	StopOnHit,
	
	// 충돌 무시하고 관통
	PassThrough
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SF_API USFHeroMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	
public:

	USFHeroMovementComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual float GetMaxSpeed() const override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "SF|Movement")
	float LeftRightMovePercent = 0.7f;
	
	UPROPERTY(EditDefaultsOnly, Category = "SF|Movement")
	float BackwardMovePercent = 0.6f;

	UPROPERTY(EditDefaultsOnly, Category = "SF|Movement")
	float CrouchMovePercent = 0.5f;

public:

	// Warp 타겟 설정 (AbilityTask에서 호출) 
	UFUNCTION(BlueprintCallable, Category = "SF|Movement|Warp")
	void SetWarpTarget(const FVector& Location, const FRotator& Rotation);

	UFUNCTION(BlueprintCallable, Category = "SF|Movement|Warp")
	void ClearWarpTarget();
	
	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	virtual void MoveAutonomous(float ClientTimeStamp, float DeltaTime, uint8 CompressedFlags, const FVector& NewAccel) override;
	
	// Host/로컬에서 PerformMovement 전 Warp 타겟 적용 
	void ApplyWarpTargetToMotionWarping();

public:
	
	// 현재 Warp 타겟 위치 (AbilityTask에서 매 틱 업데이트) 
	UPROPERTY()
	FVector CurrentWarpTargetLocation;

	UPROPERTY()
	FRotator CurrentWarpTargetRotation;

	UPROPERTY()
	bool bHasActiveWarpTarget;
	
public:
	// 충돌시 슬라이드 설정 (충돌 Response도 함께 처리)
	UFUNCTION(BlueprintCallable, Category = "SF|Movement")
	void SetSlidingMode(ESFSlidingMode NewMode);
	
protected:
	
	virtual float SlideAlongSurface(const FVector& Delta, float Time, const FVector& Normal, FHitResult& Hit, bool bHandleImpact) override;

private:
	void ApplyPassThroughCollision();
	void RestoreCollision();

public:

	UPROPERTY(BlueprintReadWrite, Category = "SF|Movement")
	ESFSlidingMode SlidingMode = ESFSlidingMode::Normal;

	// 정면 충돌 판정 각도 (이 각도 이내면 정면 충돌로 판정)
	// 0.5 = 약 60도, 0.7 = 약 45도, 0.85 = 약 30도
	UPROPERTY(EditDefaultsOnly, Category = "SF|Movement")
	float FrontalHitThreshold = 0.7f;
	
	// 슬라이딩 비활성화 (true면 충돌 시 정지)
	UPROPERTY(BlueprintReadWrite, Category = "SF|Movement")
	bool bDisableSliding = false;

	// 관통 모드에서 무시할 충돌 채널들
	UPROPERTY(EditDefaultsOnly, Category = "SF|Movement")
	TArray<TEnumAsByte<ECollisionChannel>> PassThroughChannels;

private:
	FSFCharacterNetworkMoveDataContainer SFNetworkMoveDataContainer;
	
	// 원본 Collision 정보 저장
	TMap<ECollisionChannel, ECollisionResponse> SavedCollisionResponses;

	UPROPERTY(Transient)
	TObjectPtr<UMotionWarpingComponent> CachedMotionWarpingComp;
};

// 데이터 캡처 및 복원 담당
class FSavedMove_SFCharacter : public FSavedMove_Character
{
public:
	typedef FSavedMove_Character Super;

	// 저장할 Warp 타겟 데이터
	FVector SavedWarpTargetLocation;
	FRotator SavedWarpTargetRotation;
	uint8 bSavedHasWarpTarget:1;

	FSavedMove_SFCharacter()
		: SavedWarpTargetLocation(FVector::ZeroVector)
		, SavedWarpTargetRotation(FRotator::ZeroRotator)
		, bSavedHasWarpTarget(false)
	{
	}

	virtual void Clear() override;

	// 현재 CMC 상태를 SavedMove에 저장(호출 시점: 클라이언트가 서버에 이동 데이터 전송 전)
	virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;

	// SavedMove 데이터를 CMC로 복원(호출 시점: 서버 응답 후 재시뮬레이션 직전)
	virtual void PrepMoveFor(ACharacter* C) override;
	
	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
};

class FNetworkPredictionData_Client_SFCharacter : public FNetworkPredictionData_Client_Character
{
public:
	typedef FNetworkPredictionData_Client_Character Super;

	FNetworkPredictionData_Client_SFCharacter(const UCharacterMovementComponent& ClientMovement);

	//  커스텀 SavedMove(FSavedMove_SFCharacter) 생성
	virtual FSavedMovePtr AllocateNewMove() override;
};
