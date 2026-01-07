#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "GameplayTagContainer.h"
#include "SFLockOnComponent.generated.h"

/**
 * USFLockOnComponent
 * 플레이어의 락온 타겟 탐색 및 상태 관리를 담당하는 컴포넌트
 */
UCLASS(Blueprintable, Meta = (BlueprintSpawnableComponent))
class SF_API USFLockOnComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	USFLockOnComponent(const FObjectInitializer& ObjectInitializer);

	// 매 프레임 타겟 유효성 검사 (거리, 사망 여부 등)
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 락온 시도 (성공 시 true 반환)
	UFUNCTION(BlueprintCallable, Category = "SF|LockOn")
	bool TryLockOn();

	// 락온 해제
	UFUNCTION(BlueprintCallable, Category = "SF|LockOn")
	void EndLockOn();

	// 현재 타겟 가져오기
	UFUNCTION(BlueprintPure, Category = "SF|LockOn")
	AActor* GetCurrentTarget() const { return CurrentTarget; }

	// 타겟 스위칭
	void HandleTargetSwitching(float DeltaTime);

protected:
	// 타겟 탐색 로직 (화면 중앙 가중치)
	AActor* FindBestTarget();

	// 타겟이 유효한지 검사 (거리, 시야)
	bool IsTargetValid(AActor* TargetActor) const;
	
	// 위젯 관리
	void CreateLockOnWidget();
	void DestroyLockOnWidget();

protected:
	// 토글 쿨타임 체크용 시간 변수
	double LastLockOnToggleTime = 0.0;
	
	// 현재 락온된 대상
	UPROPERTY(BlueprintReadOnly, Category = "SF|LockOn")
	TObjectPtr<AActor> CurrentTarget;

	// 락온 가능한 최대 거리
	UPROPERTY(EditDefaultsOnly, Category = "SF|LockOn")
	float LockOnDistance = 1500.0f;

	// 락온 해제 거리 (약간의 유예를 둠)
	UPROPERTY(EditDefaultsOnly, Category = "SF|LockOn")
	float LockOnBreakDistance = 1700.0f;

	// 화면 중앙 가중치 (0~1, 높을수록 중앙에 있어야 잡힘)
	UPROPERTY(EditDefaultsOnly, Category = "SF|LockOn")
	float ScreenCenterWeight = 0.6f;

	// 락온 대상 필터 태그 (Enemy 태그 등)
	UPROPERTY(EditDefaultsOnly, Category = "SF|LockOn")
	FGameplayTagContainer TargetTags;
	
	// 마우스 간섭 없는 회전을 위한 변수
	FRotator LastLockOnRotation;
	
	// 현재 타겟 스위칭(카메라 이동) 중인지 여부
	bool bIsSwitchingTarget;
	
	// ==========================================
	//  UI 설정
	// ==========================================
    
	// 에디터에서 지정할 위젯 클래스 (예: WBP_LockOnReticle)
	UPROPERTY(EditDefaultsOnly, Category = "SF|LockOn|UI")
	TSubclassOf<UUserWidget> LockOnWidgetClass;

	// 생성된 위젯 인스턴스
	UPROPERTY()
	TObjectPtr<UUserWidget> LockOnWidgetInstance;
    
	// 위젯을 띄울 소켓 이름 (예: spine_02)
	UPROPERTY(EditDefaultsOnly, Category = "SF|LockOn|UI")
	FName LockOnSocketName = FName("spine_02");

	// ==========================================
	//  타겟 스위칭 설정
	// ==========================================

	// 스위칭 감지 임계값 (스틱을 0.7 이상 밀어야 인식)
	UPROPERTY(EditDefaultsOnly, Category = "SF|LockOn|Switching")
	float SwitchInputThreshold = 0.7f;

	// 스위칭 쿨타임 (연속 변경 방지)
	UPROPERTY(EditDefaultsOnly, Category = "SF|LockOn|Switching")
	float SwitchCooldown = 0.3f;

	float CurrentSwitchCooldown = 0.0f;
	FVector2D LastInputVector = FVector2D::ZeroVector;
};