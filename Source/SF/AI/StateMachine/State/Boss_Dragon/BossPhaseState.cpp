#include "BossPhaseState.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/SFAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/SFGameplayAbility.h"
#include "AbilitySystem/Abilities/Enemy/SFEnemyAbilityInitializer.h"
#include "AI/StateMachine/SFStateMachine.h"
#include "System/SFGameInstance.h"

void UBossPhaseState::OnEnter_Implementation()
{
    Super::OnEnter_Implementation();

    if (!OwnerActor || !OwnerActor->HasAuthority())
        return;

  
    if (PhaseAbilities.Num() > 0)
    {
        GivePhaseAbilities();
    }

    
    if (StateMachine && BehaviourTag.IsValid())
    {
        if (StateMachine->OnChangeTreeDelegate.IsBound())
        {
            StateMachine->OnChangeTreeDelegate.Broadcast(BehaviourTag);
        }
    }
}

void UBossPhaseState::OnExit_Implementation()
{
    Super::OnExit_Implementation();

    if (!OwnerActor || !OwnerActor->HasAuthority())
        return;
    
    ClearPhaseAbilities();
}

void UBossPhaseState::GivePhaseAbilities()
{
    USFAbilitySystemComponent* ASC = Cast<USFAbilitySystemComponent>(
        UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwnerActor));

    if (!ASC) return;

    USFGameInstance* GI = Cast<USFGameInstance>(OwnerActor->GetWorld()->GetGameInstance());

    for (const FSFBossPhaseAbility& AbilityConfig : PhaseAbilities)
    {
        if (!AbilityConfig.AbilityClass) continue;

        FGameplayAbilitySpec Spec(AbilityConfig.AbilityClass, 1, INDEX_NONE, OwnerActor);
        FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(Spec);

        if (!Handle.IsValid()) continue;

      
        if (GI)
        {
            if (USFGameplayAbility* CDO = Cast<USFGameplayAbility>(AbilityConfig.AbilityClass->GetDefaultObject()))
            {
                if (const FAbilityBaseData* Data = GI->FindAbilityData(CDO->GetAbilityID()))
                {
                    if (FGameplayAbilitySpec* RealSpec = ASC->FindAbilitySpecFromHandle(Handle))
                    {
                        USFEnemyAbilityInitializer::ApplyAbilityData(*RealSpec, *Data);
                    }
                }
            }
        }

       
        GrantedPhaseAbilityHandles.Add({ Handle, AbilityConfig.bClearOnExit });

        
        if (AbilityConfig.bActivateOnEnter)
        {
            ASC->TryActivateAbility(Handle);
        }
    }
}

void UBossPhaseState::ClearPhaseAbilities()
{
    USFAbilitySystemComponent* ASC = Cast<USFAbilitySystemComponent>(
        UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwnerActor));

    if (!ASC) return;

    
    for (int32 i = GrantedPhaseAbilityHandles.Num() - 1; i >= 0; --i)
    {
        const FGrantedPhaseAbility& Granted = GrantedPhaseAbilityHandles[i];

        if (!Granted.bClearOnExit) continue;

        if (Granted.Handle.IsValid())
        {
            ASC->CancelAbilityHandle(Granted.Handle);
            
            ASC->ClearAbility(Granted.Handle);
        }

        GrantedPhaseAbilityHandles.RemoveAt(i);
    }
}