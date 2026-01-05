#include "SFGA_Dragon_Land.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/Enemy/Component/SFDragonMovementComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/Abilities/SFGameplayAbilityTags.h"
#include "AbilitySystem/GameplayEvent/SFGameplayEventTags.h"
#include "Character/SFCharacterGameplayTags.h"

USFGA_Dragon_Land::USFGA_Dragon_Land()
{
    AbilityTags.AddTag(SFGameplayTags::Ability_Dragon_Movement_Land);
    ActivationOwnedTags.AddTag(SFGameplayTags::Character_State_UsingAbility);

    if (HasAnyFlags(RF_ClassDefaultObject))
    {
        FAbilityTriggerData LandTrigger;
        LandTrigger.TriggerTag = SFGameplayTags::GameplayEvent_Dragon_Flight_Land;
        LandTrigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
        AbilityTriggers.Add(LandTrigger);
    }
}

void USFGA_Dragon_Land::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (!Character) 
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    USFDragonMovementComponent* MoveComp = Cast<USFDragonMovementComponent>(Character->GetCharacterMovement());
    if (!MoveComp)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    
    MoveComp->SetFlyingMode(false);
   
    FVector DropVelocity = FVector(0.f, 0.f, -LandingDownwardSpeed);

    // LaunchCharacter 파라미터 설명:
    // Arg 1: 가해질 속도 벡터
    // Arg 2 (bXYOverride): true면 기존 수평 속도를 무시하고 0으로 만듦 (수직 낙하)
    // Arg 3 (bZOverride): true면 기존 수직 속도를 무시하고 즉시 내리꽂음
    
    // bStopHorizontalMovement가 true면 XY를 0으로 덮어써서 '제자리 쿵'을 만듭니다.
    Character->LaunchCharacter(DropVelocity, bStopHorizontalMovement, true);
    
    if (!MoveComp->IsFalling())
    {
        FHitResult DummyHit;
        OnLandedCallback(DummyHit);
        return;
    }
    
    Character->LandedDelegate.AddDynamic(this, &USFGA_Dragon_Land::OnLandedCallback);
}

void USFGA_Dragon_Land::OnLandedCallback(const FHitResult& Hit)
{
    ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (Character)
    {
        Character->LandedDelegate.RemoveDynamic(this, &USFGA_Dragon_Land::OnLandedCallback);
    }
    
    if (LandMontage)
    {
        UAbilityTask_PlayMontageAndWait* MontageTask = 
            UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
                this, NAME_None, LandMontage, 1.0f, NAME_None, true
            );

        if (MontageTask)
        {
            MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
            MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageFinished);
            MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageFinished);
            MontageTask->ReadyForActivation();
        }
    }
    else
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
    }
}

void USFGA_Dragon_Land::OnMontageFinished()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void USFGA_Dragon_Land::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (Character)
    {
        Character->LandedDelegate.RemoveDynamic(this, &USFGA_Dragon_Land::OnLandedCallback);
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}