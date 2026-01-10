#include "SFGA_LockOn.h"
#include "SF/Character/Hero/Component/SFLockOnComponent.h"
#include "SF/Character/SFCharacterBase.h"
#include "GameFramework/PlayerController.h"

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
		LockOnComp->TryLockOn();
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}