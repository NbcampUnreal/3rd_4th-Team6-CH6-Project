#include "SFGA_LockOn.h"
#include "SF/Character/Hero/Component/SFLockOnComponent.h"
#include "SF/Character/SFCharacterBase.h"

USFGA_LockOn::USFGA_LockOn()
{
	// 입력 태그 설정 (Input Config에 등록된 태그와 매칭)
	// InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor; // 필요시 설정
}

void USFGA_LockOn::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	APawn* AvatarPawn = Cast<APawn>(ActorInfo->AvatarActor.Get());
	if (!AvatarPawn)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// 캐릭터에서 LockOnComponent 가져오기
	USFLockOnComponent* LockOnComp = AvatarPawn->FindComponentByClass<USFLockOnComponent>();
	if (LockOnComp)
	{
		// 락온 시도 (Toggle 로직은 컴포넌트 내부에서 처리)
		bool bSuccess = LockOnComp->TryLockOn();

		if (!bSuccess && !LockOnComp->GetCurrentTarget())
		{
			// 락온 실패했고, 기존 타겟도 없다면 -> 카메라 리셋 (정면 보기)
			// 구현 팁: Controller의 SetControlRotation 등을 사용해 구현 가능
			// 예: Controller->SetControlRotation(AvatarPawn->GetActorRotation());
		}
	}

	// 이 어빌리티는 상태를 유지하지 않고 트리거만 하고 즉시 종료됨
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}