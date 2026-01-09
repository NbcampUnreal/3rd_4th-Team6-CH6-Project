#pragma once

#include "CoreMinimal.h"
#include "SF/AbilitySystem/Abilities/SFGameplayAbility.h"
#include "SFGA_LockOn.generated.h"

/**
 * USFGA_LockOn
 * 플레이어의 락온 입력을 처리하여 LockOnComponent를 제어하는 어빌리티
 */
UCLASS()
class SF_API USFGA_LockOn : public USFGameplayAbility
{
	GENERATED_BODY()

public:
	USFGA_LockOn();

	// 어빌리티 실행 로직
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};