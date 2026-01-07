// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/SFGameplayAbility.h"
#include "USFGA_Dodge.generated.h"

/**
 * 소울류 구르기 (클라이언트 예측 + 서버 동기화 버전)
 */
UCLASS()
class SF_API USFGA_Dodge : public USFGameplayAbility
{
	GENERATED_BODY()

public:
	USFGA_Dodge(FObjectInitializer const& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	// 몽타주 종료 콜백
	UFUNCTION()
	void OnMontageFinished();

	// [계산] 구를 방향과 위치를 계산만 하는 함수
	void CalculateDodgeParameters(FVector& OutLocation, FRotator& OutRotation) const;

	// [실행] 계산된 위치로 실제 구르기(MotionWarping + Montage)를 실행하는 함수
	void ApplyDodge(const FVector& TargetLocation, const FRotator& TargetRotation);

	// [서버] 클라이언트로부터 위치 데이터를 받았을 때 실행
	void OnServerTargetDataReceived(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag ActivationTag);
	
	// Motion Warping 컴포넌트 업데이트 헬퍼
	void SetupMotionWarping(const FVector& TargetLocation, const FRotator& TargetRotation);

protected:
	// 구르기 애니메이션 몽타주
	UPROPERTY(EditDefaultsOnly, Category = "SF|Animation")
	TObjectPtr<UAnimMontage> DodgeMontage;

	// 백스텝 몽타주 (입력 없이 스페이스바 눌렀을 때)
	UPROPERTY(EditDefaultsOnly, Category = "SF|Animation")
	UAnimMontage* BackstepMontage;
	
	// 구르기 이동 거리
	UPROPERTY(EditDefaultsOnly, Category = "SF|Dodge")
	float DodgeDistance = 500.f;

	// Motion Warping 타겟 이름 (몽타주 노티파이와 일치해야 함)
	UPROPERTY(EditDefaultsOnly, Category = "SF|MotionWarping")
	FName WarpTargetName = TEXT("Dodge");

	// DodgeType 파라미터 추가 (0: 구르기, 1: 백스텝)
	void CalculateDodgeParameters(FVector& OutLocation, FRotator& OutRotation, int32& OutDodgeType) const;
	void ApplyDodge(const FVector& TargetLocation, const FRotator& TargetRotation, int32 DodgeType);
	
private:
	// 서버에서 데이터를 기다릴 때 쓰는 핸들
	FDelegateHandle ServerTargetDataDelegateHandle;
};