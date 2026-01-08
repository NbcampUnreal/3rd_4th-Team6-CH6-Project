#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "SFLockOnComponent.generated.h"

/**
 * USFLockOnComponent
 * 소울라이크 스타일의 락온(Lock-On) 시스템을 담당하는 컴포넌트
 * * [주요 기능]
 * 1. 화면 중앙 가중치(Dot Product) 기반 타겟 탐색
 * 2. 하드 락온(Hard Lock): 마우스 회전 입력을 무시하고 타겟 고정
 * 3. 타겟 스위칭(Target Switching): 입력 방향으로 타겟 변경
 * 4. 시야 가림 유예(Grace Period): 장애물에 가려져도 잠시 락온 유지
 * 5. 캐릭터 이동 제어: 락온 시 Strafing(게걸음) 모드로 전환
 */
UCLASS(Blueprintable, Meta = (BlueprintSpawnableComponent))
class SF_API USFLockOnComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	USFLockOnComponent(const FObjectInitializer& ObjectInitializer);

	// 매 프레임 로직 수행 (유효성 검사, 회전 제어 등)
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * 락온을 시도하거나 해제합니다. (Toggle)
	 * @return true: 락온 성공 또는 해제 성공 (카메라 리셋 불필요)
	 * @return false: 타겟을 찾지 못함 (카메라 리셋 필요)
	 */
	UFUNCTION(BlueprintCallable, Category = "SF|LockOn")
	bool TryLockOn();

	// 락온 강제 해제
	UFUNCTION(BlueprintCallable, Category = "SF|LockOn")
	void EndLockOn();

	// 현재 타겟 반환
	UFUNCTION(BlueprintPure, Category = "SF|LockOn")
	AActor* GetCurrentTarget() const { return CurrentTarget; }

protected:
	// ==========================================
	//  틱(Tick) 내부 로직 분리 (리팩토링)
	// ==========================================
	
	// 1. 타겟 유효성 검사 (거리, 시야 가림 유예 처리)
	void UpdateLogic_TargetValidation(float DeltaTime);

	// 2. 타겟 스위칭 입력 처리 (마우스/스틱)
	void HandleTargetSwitching(float DeltaTime);

	// 3. 카메라 회전 제어 (Hard Lock 및 스위칭 보간)
	void UpdateLogic_CameraRotation(float DeltaTime);

	// 4. 캐릭터 회전 제어 (카메라 방향과 동기화)
	void UpdateLogic_CharacterRotation(float DeltaTime);

	// 5. 위젯 위치 업데이트 (선택 사항)
	void UpdateLogic_WidgetPosition(float DeltaTime);

protected:
	// ==========================================
	//  내부 유틸리티 함수
	// ==========================================

	// 최적의 타겟 탐색 (화면 중앙 우선)
	AActor* FindBestTarget();
	
	// 기본적인 타겟 유효성 검사 (거리, 사망 여부 - 시야 체크 제외)
	bool IsTargetValidBasic(AActor* TargetActor) const;

	// UI 위젯 생성 및 파괴
	void CreateLockOnWidget();
	void DestroyLockOnWidget();

protected:
	// ==========================================
	//  변수 및 설정
	// ==========================================

	// 현재 락온된 대상
	UPROPERTY(BlueprintReadOnly, Category = "SF|LockOn")
	TObjectPtr<AActor> CurrentTarget;

	// 락온 탐색 거리
	UPROPERTY(EditDefaultsOnly, Category = "SF|LockOn")
	float LockOnDistance = 1500.0f;

	// 락온 해제 거리 (탐색 거리보다 약간 길게 설정하여 잦은 끊김 방지)
	UPROPERTY(EditDefaultsOnly, Category = "SF|LockOn")
	float LockOnBreakDistance = 1700.0f;

	// 화면 중앙 가중치 (0~1, 클수록 중앙에 있는 적 우선)
	UPROPERTY(EditDefaultsOnly, Category = "SF|LockOn")
	float ScreenCenterWeight = 0.6f;

	// 타겟 필터 태그 (Enemy 등)
	UPROPERTY(EditDefaultsOnly, Category = "SF|LockOn")
	FGameplayTagContainer TargetTags;
	
	// 달리기 상태 태그
	UPROPERTY(EditDefaultsOnly, Category = "SF|LockOn")
	FGameplayTag SprintTag;

	// ------------------------------------------
	// 시야 가림(Occlusion) 유예 설정
	// ------------------------------------------
	
	// 장애물에 가려져도 락온을 유지하는 시간 (초)
	UPROPERTY(EditDefaultsOnly, Category = "SF|LockOn")
	float LostTargetMemoryTime = 1.0f;

	// 타겟이 시야에서 사라진 누적 시간
	float TimeSinceTargetHidden = 0.0f;

	// ------------------------------------------
	// 카메라 및 스위칭 설정
	// ------------------------------------------

	// 마우스 간섭 없는 회전을 위해 마지막 프레임의 회전값 저장
	FRotator LastLockOnRotation;

	// 현재 타겟 스위칭(카메라 이동) 중인지 여부
	bool bIsSwitchingTarget = false;

	// 타겟 스위칭 시 카메라 이동 속도 (높을수록 빠름)
	UPROPERTY(EditAnywhere, Category = "SF|LockOn|Switching")
	float TargetSwitchInterpSpeed = 15.0f;

	// 스위칭 입력 감도 (낮을수록 민감)
	UPROPERTY(EditDefaultsOnly, Category = "SF|LockOn|Switching")
	float SwitchInputThreshold = 0.5f;
	
	// 스위칭 시 허용할 최대 각도 (내적값)
	UPROPERTY(EditDefaultsOnly, Category = "SF|LockOn|Switching")
	float SwitchAngularLimit = 0.5f;
	
	// 스위칭 쿨타임 (초)
	UPROPERTY(EditDefaultsOnly, Category = "SF|LockOn|Switching")
	float SwitchCooldown = 0.3f;

	float CurrentSwitchCooldown = 0.0f;
	
	// 토글(On/Off) 연타 방지용 마지막 시간
	double LastLockOnToggleTime = 0.0;

	// ------------------------------------------
	// UI 설정
	// ------------------------------------------

	// 락온 위젯 클래스 (BP에서 할당)
	UPROPERTY(EditDefaultsOnly, Category = "SF|LockOn|UI")
	TSubclassOf<UUserWidget> LockOnWidgetClass;

	// 생성된 위젯 인스턴스
	UPROPERTY()
	TObjectPtr<UUserWidget> LockOnWidgetInstance;

	// 위젯을 고정할 타겟의 소켓 이름
	UPROPERTY(EditDefaultsOnly, Category = "SF|LockOn|UI")
	FName LockOnSocketName = FName("spine_02");
};