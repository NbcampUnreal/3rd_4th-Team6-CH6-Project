#include "SFGA_Dragon_TakeOff.h"
#include "GameFramework/Character.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/Abilities/SFGameplayAbilityTags.h"
#include "AbilitySystem/GameplayEvent/SFGameplayEventTags.h"
#include "Character/SFCharacterGameplayTags.h"
#include "Character/Enemy/Component/SFDragonMovementComponent.h"

USFGA_Dragon_TakeOff::USFGA_Dragon_TakeOff()
{
    AbilityTags.AddTag(SFGameplayTags::Ability_Dragon_Movement_Takeoff);
    ActivationOwnedTags.AddTag(SFGameplayTags::Character_State_UsingAbility);
}

void USFGA_Dragon_TakeOff::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (Character)
    {
        USFDragonMovementComponent* MoveComp = Cast<USFDragonMovementComponent>(Character->GetCharacterMovement());
        if (MoveComp)
        {
            MoveComp->SetFlyingMode(true);
        }
    }

    if (TakeOffMontage)
    {
        UAbilityTask_PlayMontageAndWait* MontageTask =
            UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
                this, NAME_None, TakeOffMontage, 1.0f, NAME_None, false
            );

        if (MontageTask)
        {
            MontageTask->OnCompleted.AddDynamic(this, &ThisClass::EndAbilityDefault);
            MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::EndAbilityDefault);
            MontageTask->OnCancelled.AddDynamic(this, &ThisClass::EndAbilityDefault);
            MontageTask->ReadyForActivation();
        }
    }
    else
    {
        EndAbilityDefault();
        return;
    }
}
void USFGA_Dragon_TakeOff::EndAbilityDefault()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void USFGA_Dragon_TakeOff::EndAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility, bool bWasCancelled)
{
    if (!bWasCancelled)
    {
        FGameplayEventData EventData;
        UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
            GetAvatarActorFromActorInfo(),
            SFGameplayTags::GameplayEvent_Dragon_TakeOff_Completed,
            EventData
        );
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}