// Fill out your copyright notice in the Description page of Project Settings.


#include "SFGA_EnemyDeath.h"

#include "AbilitySystem/GameplayEvent/SFGameplayEventTags.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/SFCharacterGameplayTags.h"

USFGA_EnemyDeath::USFGA_EnemyDeath()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	bServerRespectsRemoteAbilityCancellation = true;
	
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = SFGameplayTags::GameplayEvent_Death;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);



	// [수정] 중요: 이 어빌리티가 활성화된 동안 클라이언트도 'Dead' 태그를 가지게 함
	// 이게 있어야 클라이언트 애니메이션이 'Dead' 상태를 인지함
	ActivationOwnedTags.AddTag(SFGameplayTags::Character_State_Dead);

}

void USFGA_EnemyDeath::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AActor* Avatar = GetAvatarActorFromActorInfo();


	if (ACharacter* Character = Cast<ACharacter>(Avatar))
	{
		// 캡슐 컴포넌트 가져오기
		if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
		{
			// 충돌 반응을 아예 없애버림 (물리적 충돌 X, Trace 감지 X)
			Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Capsule->SetCollisionResponseToAllChannels(ECR_Ignore);
		}

		// 이동 컴포넌트도 정지시킴 (혹시 미끄러지는 현상 방지)
		if (UCharacterMovementComponent* CMC = Character->GetCharacterMovement())
		{
			CMC->StopMovementImmediately();
			CMC->DisableMovement();
		}
	}
	
	if (!Avatar)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (Avatar->HasAuthority())
	{
		FTimerDelegate TimerDel;
		TimerDel.BindUObject(this, &ThisClass::DeathEventAfterDelay);
		Avatar->GetWorldTimerManager().SetTimer(EventTimerHandle, TimerDel, EventTime, false);
	}
}

void USFGA_EnemyDeath::DeathEventAfterDelay()
{
	DestroyEnemy();
}

void USFGA_EnemyDeath::DestroyEnemy()
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (Avatar && Avatar->HasAuthority())
	{
		Avatar->Destroy();
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}


