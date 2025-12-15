#include "SFGA_EnemyHitReaction.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/Attributes/SFPrimarySet.h"
#include "AbilitySystem/GameplayCues/SFGameplayCueTags.h"
#include "AbilitySystem/GameplayEffect/SFGameplayEffectContext.h"
#include "AbilitySystem/GameplayEvent/SFGameplayEventTags.h"
#include "Character/SFCharacterGameplayTags.h"
#include "GameFramework/Character.h"

USFGA_EnemyHitReaction::USFGA_EnemyHitReaction()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    bServerRespectsRemoteAbilityCancellation = true;

    FAbilityTriggerData TriggerData;
    TriggerData.TriggerTag   = SFGameplayTags::GameplayEvent_HitReaction;
    TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
    AbilityTriggers.Add(TriggerData);

    ActivationOwnedTags.AddTag(SFGameplayTags::Character_State_Hit);
}

void USFGA_EnemyHitReaction::ActivateAbility(const FGameplayAbilitySpecHandle Handle,    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
        return;
    }

    if (!TriggerEventData)
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (!Character)
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
        return;
    }
    
   
    float Damage = TriggerEventData->EventMagnitude;
    if (Damage <= 0.f)
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    FVector HitLocation = ExtractHitLocationFromEvent(TriggerEventData);
    FVector AttackDir   = ExtractHitDirectionFromEvent(TriggerEventData);


    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (!ASC)
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
        return;
    }

    const USFPrimarySet* PrimarySet = ASC->GetSet<USFPrimarySet>();
    if (!PrimarySet || PrimarySet->GetHealth() <= 0.f)
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    // 파티클 먼저 실행 (항상 실행됨)
    FGameplayCueParameters CueParams;
    CueParams.Location = HitLocation;

    const USFPrimarySet* Set = ASC->GetSet<USFPrimarySet>();
    if (Set)
    {
        float MaxHealth = Set->GetMaxHealth();
        float DamageRatio = Damage / MaxHealth;

        FGameplayTag TypeTag = (DamageRatio > 0.2f) ? SFGameplayTags::GameplayCue_HitReaction_Heavy : SFGameplayTags::GameplayCue_HitReaction_Light;

        CueParams.AggregatedSourceTags.AddTag(TypeTag);
    }

    ASC->ExecuteGameplayCue(SFGameplayTags::GameplayCue_HitReaction_Type_Enemy, CueParams);

    // 몽타주가 이미 재생 중이면 몽타주만 스킵하고 종료
    if (MontageTask && MontageTask->IsActive())
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    // 몽타주 재생
    FVector Forward = Character->GetActorForwardVector();
    float Dot = FVector::DotProduct(Forward, AttackDir);  // 1 = 정면, -1 = 뒤

    UAnimMontage* SelectedMontage = (Dot < 0.f) ? BackHitMontage : FrontHitMontage;

    if (SelectedMontage)
    {
        MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this, NAME_None, SelectedMontage);

        if (MontageTask)
        {
            MontageTask->OnCompleted.AddDynamic(this, &USFGA_EnemyHitReaction::OnMontageCompleted);
            MontageTask->OnInterrupted.AddDynamic(this, &USFGA_EnemyHitReaction::OnMontageInterrupted);
            MontageTask->OnCancelled.AddDynamic(this, &USFGA_EnemyHitReaction::OnMontageCancelled);

            MontageTask->ReadyForActivation();
            return;
        }
    }

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}


FVector USFGA_EnemyHitReaction::ExtractHitLocationFromEvent(const FGameplayEventData* EventData) const
{
    if (!EventData) return FVector::ZeroVector;

    const FGameplayEffectContextHandle& Ctx = EventData->ContextHandle;
    if (!Ctx.IsValid()) return FVector::ZeroVector;

    // 2) HitResult fallback
    if (const FHitResult* HR = Ctx.GetHitResult())
    {
        return HR->ImpactPoint;
    }

    return FVector::ZeroVector;
}

FVector USFGA_EnemyHitReaction::ExtractHitDirectionFromEvent(const FGameplayEventData* EventData) const
{
    if (!EventData) return FVector::ZeroVector;

    const FGameplayEffectContextHandle& Ctx = EventData->ContextHandle;
    if (!Ctx.IsValid()) return FVector::ZeroVector;

    AActor* Avatar = GetAvatarActorFromActorInfo();
    if (!Avatar) return FVector::ZeroVector;

   
    if (const FHitResult* HR = Ctx.GetHitResult())
    {
        FVector Dir = HR->ImpactPoint - Avatar->GetActorLocation();  
        Dir.Z = 0.f;
        if (!Dir.IsNearlyZero())
            return Dir.GetSafeNormal();
    }

    if (AActor* Instigator = const_cast<AActor*>(EventData->Instigator.Get()))
    {
        FVector Dir = Instigator->GetActorLocation() - Avatar->GetActorLocation();
        Dir.Z = 0.f;
        if (!Dir.IsNearlyZero())
            return Dir.GetSafeNormal();
    }

    return FVector::ZeroVector;
}


void USFGA_EnemyHitReaction::OnMontageCompleted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void USFGA_EnemyHitReaction::OnMontageCancelled()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void USFGA_EnemyHitReaction::OnMontageInterrupted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void USFGA_EnemyHitReaction::EndAbility( const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    if (MontageTask)
    {
        MontageTask->EndTask();
        MontageTask = nullptr;
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
