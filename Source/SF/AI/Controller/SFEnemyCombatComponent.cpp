// Fill out your copyright notice in the Description page of Project Settings.

#include "SFEnemyCombatComponent.h"
#include "TimerManager.h"
#include "AIController.h"
#include "AbilitySystemGlobals.h"
#include "SFBaseAIController.h"
#include "AbilitySystem/SFAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/Enemy/SFCombatSet_Enemy.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"
#include "AI/SFAIGameplayTags.h"
#include "Character/SFCharacterBase.h"         
#include "Character/SFCharacterGameplayTags.h"
#include "Character/Enemy/SFEnemyGameplayTags.h"

USFEnemyCombatComponent::USFEnemyCombatComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void USFEnemyCombatComponent::InitializeCombatComponent()
{
    Super::InitializeCombatComponent();

    if (!CachedASC) return;

    const UAttributeSet* Set = CachedASC->GetAttributeSet(USFCombatSet_Enemy::StaticClass());
    CachedCombatSet = const_cast<USFCombatSet_Enemy*>(Cast<USFCombatSet_Enemy>(Set));

    if (CachedCombatSet)
    {
        UpdatePerceptionConfig();
    }
    
    AAIController* AIC = GetController<AAIController>();
    if (AIC)
    {
        if (UAIPerceptionComponent* PerceptionComp = AIC->GetPerceptionComponent())
        {
            PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &USFEnemyCombatComponent::OnTargetPerceptionUpdated);
            PerceptionComp->OnPerceptionUpdated.AddDynamic(this, &USFEnemyCombatComponent::OnPerceptionUpdated);
        }
    }

    StartTargetEvaluationTimer();
}

void USFEnemyCombatComponent::StartTargetEvaluationTimer()
{
    AAIController* AIC = GetController<AAIController>();
    if (!AIC) return;

    UWorld* World = AIC->GetWorld();
    if (!World) return;

    if (TargetEvaluationTimerHandle.IsValid())
    {
        World->GetTimerManager().ClearTimer(TargetEvaluationTimerHandle);
    }

    World->GetTimerManager().SetTimer(
        TargetEvaluationTimerHandle,
        this,
        &USFEnemyCombatComponent::OnTargetEvaluationTimer,
        TargetEvaluationInterval,
        true
    );
}

void USFEnemyCombatComponent::StopTargetEvaluationTimer()
{
    AAIController* AIC = GetController<AAIController>();
    if (!AIC) return;

    UWorld* World = AIC->GetWorld();
    if (!World) return;

    if (TargetEvaluationTimerHandle.IsValid())
    {
        World->GetTimerManager().ClearTimer(TargetEvaluationTimerHandle);
        TargetEvaluationTimerHandle.Invalidate();
    }
}

void USFEnemyCombatComponent::OnTargetEvaluationTimer()
{
    EvaluateTarget();
    UpdateCombatRangeTags();
}

void USFEnemyCombatComponent::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor || !Actor->ActorHasTag(FName("Player"))) return;
    
    if (!IsValidTarget(Actor)) 
    {
        if (CurrentTarget == Actor)
        {
            EvaluateTarget();
        }
        return;
    }

    AAIController* AIC = GetController<AAIController>();
    if (!AIC) return;
    
    if (CurrentTarget && CurrentTarget != Actor)
    {
        APawn* MyPawn = AIC->GetPawn();
        if (MyPawn)
        {
            if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(MyPawn))
            {
                if (ASC->HasMatchingGameplayTag(SFGameplayTags::Character_State_UsingAbility))
                {
                    return;
                }
            }
        }
    }

    if (Stimulus.WasSuccessfullySensed())
    {
        EvaluateTarget();
    }
    else if (CurrentTarget == Actor)
    {
        EvaluateTarget();
    }
}

void USFEnemyCombatComponent::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    EvaluateTarget();
}

void USFEnemyCombatComponent::EvaluateTarget()
{
    AAIController* AIC = GetController<AAIController>();
    if (!AIC) return;

    UAIPerceptionComponent* PerceptionComp = AIC->GetPerceptionComponent();
    if (!PerceptionComp) return;

    TArray<AActor*> PerceivedActors;
    PerceptionComp->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActors);

    AActor* BestTarget = nullptr;
    float BestScore = -1.f;

    for (AActor* Actor : PerceivedActors)
    {
        if (!Actor->ActorHasTag(FName("Player"))) continue;
        
        if (!IsValidTarget(Actor)) continue;

        float Score = CalculateTargetScore(Actor);
        if (Score > BestScore)
        {
            BestScore = Score;
            BestTarget = Actor;
        }
    }
    
    UpdateTargetActor(BestTarget);
}

bool USFEnemyCombatComponent::IsValidTarget(AActor* Target) const
{
    if (!Target) return false;
    
    if (Target->IsPendingKillPending()) return false;

    ASFCharacterBase* TargetChar = Cast<ASFCharacterBase>(Target);
    if (!TargetChar) return false;

    UAbilitySystemComponent* ASC = TargetChar->GetSFAbilitySystemComponent();
    if (ASC)
    {
        if (ASC->HasMatchingGameplayTag(SFGameplayTags::Character_State_Dead) ||
            ASC->HasMatchingGameplayTag(SFGameplayTags::Character_State_Downed))
        {
            return false;
        }
    }

    return true;
}

float USFEnemyCombatComponent::CalculateTargetScore(AActor* Target) const
{
    if (!Target) return -1.f;
    APawn* MyPawn = GetOwnerPawn();
    if (!MyPawn) return -1.f;

    const float Distance = FVector::Dist(MyPawn->GetActorLocation(), Target->GetActorLocation());
    float Score = FMath::Clamp(1000.f - (Distance / 10.f), 0.f, 1000.f);

    FVector ToTarget = (Target->GetActorLocation() - MyPawn->GetActorLocation()).GetSafeNormal();
    float Dot = FVector::DotProduct(MyPawn->GetActorForwardVector(), ToTarget);
    Score += (Dot * 100.f);

    return Score;
}

void USFEnemyCombatComponent::UpdateCombatRangeTags()
{
    if (!CachedASC || !CachedCombatSet || !CurrentTarget || !IsValidTarget(CurrentTarget))
    {
        SetGameplayTagStatus(SFGameplayTags::AI_Range_Melee, false);
        SetGameplayTagStatus(SFGameplayTags::AI_Range_Guard, false);
        return;
    }

    APawn* MyPawn = GetOwnerPawn();
    if (!MyPawn) return;

    float Distance = FVector::Dist(MyPawn->GetActorLocation(), CurrentTarget->GetActorLocation());

    SetGameplayTagStatus(SFGameplayTags::AI_Range_Melee, Distance <= CachedCombatSet->GetMeleeRange());
    SetGameplayTagStatus(SFGameplayTags::AI_Range_Guard, Distance <= CachedCombatSet->GetGuardRange());
}

void USFEnemyCombatComponent::UpdatePerceptionConfig()
{
    if (!CachedCombatSet) return;

    AAIController* AIC = Cast<AAIController>(GetOwner());
    if (!AIC || !AIC->GetPerceptionComponent()) return;

    UAIPerceptionComponent* Perception = AIC->GetPerceptionComponent();
    FAISenseID SightID = UAISense::GetSenseID<UAISense_Sight>();
    UAISenseConfig_Sight* SightConfig = Cast<UAISenseConfig_Sight>(Perception->GetSenseConfig(SightID));

    if (SightConfig)
    {
        SightConfig->SightRadius = CachedCombatSet->GetSightRadius();
        SightConfig->LoseSightRadius = CachedCombatSet->GetLoseSightRadius();
        Perception->ConfigureSense(*SightConfig);
    }
}