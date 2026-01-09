// Fill out your copyright notice in the Description page of Project Settings.


#include "SFGA_EnemyParriedStagger.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/Abilities/SFGameplayAbilityTags.h"
#include "AbilitySystem/GameplayEvent/SFGameplayEventTags.h"
#include "Character/SFCharacterGameplayTags.h"

USFGA_EnemyParriedStagger::USFGA_EnemyParriedStagger()
{

	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	
	ActivationOwnedTags.AddTag(SFGameplayTags::Character_State_Parried);

	BlockAbilitiesWithTag.AddTag(SFGameplayTags::Character_State_Attacking);
	
	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = SFGameplayTags::GameplayEvent_Parried;
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
	
}

void USFGA_EnemyParriedStagger::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                const FGameplayAbilityActorInfo* ActorInfo, 
                                                const FGameplayAbilityActivationInfo ActivationInfo,
                                                const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    // Server-side logic
    if (ActorInfo->IsNetAuthority())
    {
        if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
        {
            EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
            return;
        }

        // Cancel attacking abilities FIRST
        if (CancelTags.Num() > 0)
        {
            ActorInfo->AbilitySystemComponent->CancelAbilities(&CancelTags, nullptr, this);
        }
    }

    if (!ParriedStaggerMontage)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();
    if (!AnimInstance)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }
	
    AnimInstance->Montage_Stop(0.0f);
	
    GetWorld()->GetTimerManager().SetTimerForNextTick([this, Handle, ActorInfo, ActivationInfo]()
    {
        if (!IsActive())
        {
            return;
        }

        UAnimInstance* AnimInst = ActorInfo->GetAnimInstance();
        if (!AnimInst || !ParriedStaggerMontage)
        {
            EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
            return;
        }
    	
        float MontageLength = AnimInst->Montage_Play(ParriedStaggerMontage, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);
        
        if (MontageLength > 0.0f)
        {
         
            MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
                this,
                NAME_None,
                ParriedStaggerMontage,
                1.0f,
                NAME_None,
                false,  
                1.0f,
                0.0f,
                false
            );

            if (MontageTask)
            {
                MontageTask->OnCompleted.AddDynamic(this, &USFGA_EnemyParriedStagger::OnMontageCompleted);
                MontageTask->OnInterrupted.AddDynamic(this, &USFGA_EnemyParriedStagger::OnMontageInterrupted);
                MontageTask->OnCancelled.AddDynamic(this, &USFGA_EnemyParriedStagger::OnMontageCancelled);
                MontageTask->ReadyForActivation();
            }
        }
        else
        {
            EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        }
    });
}

void USFGA_EnemyParriedStagger::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{

	if (bWasCancelled && ParriedStaggerMontage)
	{
		if (UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance())
		{
			if (AnimInstance->Montage_IsPlaying(ParriedStaggerMontage))
			{
				AnimInstance->Montage_Stop(0.2f, ParriedStaggerMontage);
			}
		}
	}
	
	if (MontageTask)
	{
		MontageTask->EndTask();
		MontageTask = nullptr;
	}
    
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void USFGA_EnemyParriedStagger::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void USFGA_EnemyParriedStagger::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void USFGA_EnemyParriedStagger::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}