#include "SFGA_PhaseIntro.h"
#include "AbilitySystemComponent.h"
#include "TimerManager.h"
#include "AbilitySystem/GameplayCues/SFGameplayCueTags.h"
#include "AI/SFAIGameplayTags.h"
#include "Character/SFCharacterGameplayTags.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

USFGA_PhaseIntro::USFGA_PhaseIntro()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

    ActivationOwnedTags.AddTag(SFGameplayTags::Character_State_Invulnerable);
    ActivationOwnedTags.AddTag(SFGameplayTags::Character_State_UsingAbility);
    ActivationOwnedTags.AddTag(SFGameplayTags::Character_State_PhaseIntro);
}

void USFGA_PhaseIntro::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        FGameplayTagContainer CancelWithTags;
        CancelWithTags.AddTag(SFGameplayTags::Character_State_UsingAbility);
        
        
        ASC->CancelAbilities(&CancelWithTags, nullptr, this);
    }

    if (IntroSteps.IsEmpty())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }
    
    CurrentStepIndex = 0;
    PlayNextStep();
    FGameplayCueParameters CueParams;
    CueParams.EffectCauser = GetAvatarActorFromActorInfo();
    CueParams.Location = GetAvatarActorFromActorInfo()->GetActorLocation();
    
    GetAbilitySystemComponentFromActorInfo()->AddGameplayCue(SFGameplayTags::GameplayCue_Dragon_PhaseIntro, FGameplayCueParameters());
}
void USFGA_PhaseIntro::PlayNextStep()
{

    if (!IntroSteps.IsValidIndex(CurrentStepIndex))
    {
      
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    const FSFPhaseIntroStep& CurrentStep = IntroSteps[CurrentStepIndex];
    
    if (CurrentMontageTask)
    {
        CurrentMontageTask->EndTask();
        CurrentMontageTask = nullptr;
    }

    if (CurrentStep.Montage)
    {
     
        CurrentMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this, NAME_None, CurrentStep.Montage, CurrentStep.PlayRate, CurrentStep.StartSection
        );

        if (CurrentMontageTask)
        {
            CurrentMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnStepFinished);
            CurrentMontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnStepFinished);
            CurrentMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnStepFinished);
            CurrentMontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnStepFinished);

            CurrentMontageTask->ReadyForActivation();
        }
        else
        {
        
            OnStepFinished();
        }
    }
    else
    {
   
        OnStepFinished();
    }
}

void USFGA_PhaseIntro::OnStepFinished()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
        return;
    }
    
    if (CurrentMontageTask)
    {
        CurrentMontageTask->EndTask();
        CurrentMontageTask = nullptr;
    }

    const FSFPhaseIntroStep FinishedStep = IntroSteps[CurrentStepIndex];
    CurrentStepIndex++;
    
    if (FinishedStep.PostWaitTime > 0.0f)
    {
        
        if (World->GetTimerManager().IsTimerActive(StepTimerHandle))
        {
            World->GetTimerManager().ClearTimer(StepTimerHandle);
        }

        World->GetTimerManager().SetTimer(
            StepTimerHandle,
            FTimerDelegate::CreateUObject(this, &ThisClass::PlayNextStep),
            FinishedStep.PostWaitTime, false
        );
    }
    else
    {
        PlayNextStep();
    }

    GetAbilitySystemComponentFromActorInfo()->RemoveGameplayCue(SFGameplayTags::GameplayCue_Dragon_PhaseIntro);
}

void USFGA_PhaseIntro::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
   
    if (!bWasCancelled)
    {
        for (const TSubclassOf<UGameplayEffect>& EffectClass : PermanentEffects)
        {
            if (EffectClass)
            {
                FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(EffectClass);
                if (SpecHandle.IsValid())
                {
                    ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
                }
            }
        }
    }

   
    if (CurrentMontageTask)
    {
        CurrentMontageTask->EndTask();
        CurrentMontageTask = nullptr;
    }

    if (GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(StepTimerHandle))
    {
        GetWorld()->GetTimerManager().ClearTimer(StepTimerHandle);
    }
    
   if (GetAbilitySystemComponentFromActorInfo()->HasMatchingGameplayTag(SFGameplayTags::GameplayCue_Dragon_PhaseIntro))
   {
       GetAbilitySystemComponentFromActorInfo()->RemoveGameplayCue(SFGameplayTags::GameplayCue_Dragon_PhaseIntro);
   }
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
