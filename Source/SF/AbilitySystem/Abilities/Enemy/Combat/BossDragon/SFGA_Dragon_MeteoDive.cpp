#include "SFGA_Dragon_MeteoDive.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/Enemy/Component/SFDragonMovementComponent.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionMoveToForce.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayTag.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "DrawDebugHelpers.h"
#include "Character/SFCharacterGameplayTags.h"
#include "Character/Enemy/Component/Boss_Dragon/SFDragonGameplayTags.h"
#include "Engine/OverlapResult.h"


USFGA_Dragon_MeteorDive::USFGA_Dragon_MeteorDive()
{
    AbilityTags.AddTag(SFGameplayTags::Ability_Dragon_DiveAttack);
    ActivationOwnedTags.AddTag(SFGameplayTags::Character_State_UsingAbility);
    ActivationOwnedTags.AddTag(SFGameplayTags::Character_State_Invulnerable);

    bIsCancelable = false;
}

void USFGA_Dragon_MeteorDive::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    
    UAbilityTask_WaitDelay* WaitTask = UAbilityTask_WaitDelay::WaitDelay(this, HoverDuration);
    if (WaitTask)
    {
        WaitTask->OnFinish.AddDynamic(this, &ThisClass::OnHoverTimeFinished);
        WaitTask->ReadyForActivation();
    }
    else
    {
        OnHoverTimeFinished();
    }
}

void USFGA_Dragon_MeteorDive::OnHoverTimeFinished()
{
    
    LaunchDiveAttack();
}

void USFGA_Dragon_MeteorDive::LaunchDiveAttack()
{
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (!OwnerCharacter) return;

    TargetLandLocation = MapCenterLocation;

    FVector StartLocation = OwnerCharacter->GetActorLocation();
    float Distance = FVector::Dist(StartLocation, TargetLandLocation);
    float Duration = (DiveSpeed > 0.f) ? (Distance / DiveSpeed) : 0.5f;
    if (Duration < 0.1f) Duration = 0.1f;

    FRotator LookAtRotation = (TargetLandLocation - StartLocation).Rotation();
    
    FRotator NewRotation = LookAtRotation; 

    OwnerCharacter->SetActorRotation(NewRotation);
    
    if (AController* OwnerController = OwnerCharacter->GetController())
    {
        OwnerController->SetControlRotation(NewRotation);
    }
    
    if (DiveLoopMontage)
    {
        OwnerCharacter->PlayAnimMontage(DiveLoopMontage);
    }
    
    UAbilityTask_ApplyRootMotionMoveToForce* DiveTask = UAbilityTask_ApplyRootMotionMoveToForce::ApplyRootMotionMoveToForce(
            this, 
            TEXT("MeteorDive"), 
            TargetLandLocation, 
            Duration, 
            true, 
            EMovementMode::MOVE_Flying, 
            false, 
            nullptr,
            ERootMotionFinishVelocityMode::SetVelocity, 
            FVector::ZeroVector, 
            0.0f
        );

    if (DiveTask)
    {
        DiveTask->OnTimedOutAndDestinationReached.AddDynamic(this, &ThisClass::OnDiveFinished);
        DiveTask->OnTimedOut.AddDynamic(this, &ThisClass::OnDiveFinished);
        DiveTask->ReadyForActivation();
    }
}

void USFGA_Dragon_MeteorDive::OnDiveFinished()
{
    ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (!Character) return;
    
    if (DiveLoopMontage)
    {
        Character->StopAnimMontage(DiveLoopMontage);
    }
  
    if (USFDragonMovementComponent* MoveComp = Cast<USFDragonMovementComponent>(Character->GetCharacterMovement()))
    {
        MoveComp->SetFlyingMode(false); 
        MoveComp->StopMovementImmediately();
    }
    
    FRotator CurrentRot = Character->GetActorRotation();
    FRotator UprightRot = FRotator(0.0f, CurrentRot.Yaw, 0.0f); 
    
    Character->SetActorRotation(UprightRot);
    
    if (AController* Controller = Character->GetController())
    {
        Controller->SetControlRotation(UprightRot);
    }

    FVector ImpactLocation = Character->GetActorLocation();


    if (ImpactCueTag.IsValid())
    {
        FGameplayCueParameters CueParams;
        CueParams.Location = ImpactLocation;
        CueParams.EffectCauser = Character;
        CueParams.RawMagnitude = ImpactRadius;
        GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(ImpactCueTag, CueParams);
    }

    ApplyImpactDamage(ImpactLocation);
    
    if (bShowDebugSphere) DrawDebugSphere(GetWorld(), ImpactLocation, ImpactRadius, 32, FColor::Red, false, 5.0f);

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void USFGA_Dragon_MeteorDive::ApplyImpactDamage(const FVector& ImpactLocation)
{
    UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
    if (!SourceASC) return;

    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetAvatarActorFromActorInfo());
    
    FCollisionObjectQueryParams ObjectQueryParams;
    ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);       
    ObjectQueryParams.AddObjectTypesToQuery(ECC_PhysicsBody); 
    
    float CheckRadius = (ImpactRadius > 0.1f) ? ImpactRadius : 500.0f; 

    bool bHit = GetWorld()->OverlapMultiByObjectType(
        OverlapResults,
        ImpactLocation,
        FQuat::Identity,
        ObjectQueryParams,
        FCollisionShape::MakeSphere(CheckRadius), 
        QueryParams
    );

    if (bHit)
    {
        for (const FOverlapResult& Result : OverlapResults)
        {
            AActor* HitActor = Result.GetActor();
            if (!HitActor) continue;
            
            FCollisionQueryParams LoSParams = QueryParams;
            LoSParams.AddIgnoredActor(HitActor);

            FVector HeightOffset = FVector(0.0f, 0.0f, 50.0f);
            FVector TraceStart = ImpactLocation + HeightOffset;
            FVector TraceEnd   = HitActor->GetActorLocation() + HeightOffset;

            FHitResult LoS;
    
            bool bIsBlocked = GetWorld()->LineTraceSingleByChannel(LoS, TraceStart, TraceEnd, ECC_Visibility, LoSParams);

            if (!bIsBlocked)
            {
                
                FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
                
                FHitResult HitResult;
                HitResult.Location = ImpactLocation;
                HitResult.ImpactPoint = HitActor->GetActorLocation();
                HitResult.Normal = (HitActor->GetActorLocation() - ImpactLocation).GetSafeNormal();
                HitResult.HitObjectHandle = FActorInstanceHandle(HitActor);

                Context.AddHitResult(HitResult, true);

                ApplyDamageToTarget(HitActor, Context);
                ApplyKnockBackToTarget(HitActor, HitActor->GetActorLocation());
            }
        }
    }
}

void USFGA_Dragon_MeteorDive::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}