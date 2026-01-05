#include "Character/Enemy/Component/SFDragonMovementComponent.h"
#include "GameFramework/Character.h"
#include "AbilitySystemGlobals.h" 
#include "AbilitySystemComponent.h"
#include "Boss_Dragon/SFDragonGameplayTags.h"
#include "Character/SFCharacterGameplayTags.h" 

USFDragonMovementComponent::USFDragonMovementComponent()
{
    MaxAcceleration = HeavyMaxAcceleration;          
    BrakingDecelerationWalking = HeavyBrakingDeceleration; 
    
    // [설정] 평상시 비행 감속도 (어빌리티에서 0으로 조절해서 씀)
    BrakingDecelerationFlying = 3000.0f; 
    
    GroundFriction = 8.0f; 

    RotationRate = FRotator(0.0f, 120.0f, 0.0f);
    bOrientRotationToMovement = true;
  
    bEnablePhysicsInteraction = false; 
    Mass = 50000.0f; 
    AirControl = 0.2f; 
    
    bIsSprinting = false;
    
    // [수정 1] 보스는 남을 피하지 않습니다. (밀림 방지)
    bUseRVOAvoidance = false; 
    AvoidanceWeight = 0.0f; 
    // AvoidanceConsiderationRadius = 800.0f; 
}

void USFDragonMovementComponent::InitializeMovementComponent()
{
    Super::InitializeMovementComponent();
    
    
    bUseRVOAvoidance = false;
    
    if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner()))
    {
    
        ASC->RemoveLooseGameplayTag(SFGameplayTags::Dragon_Movement_Flying);
        ASC->AddLooseGameplayTag(SFGameplayTags::Dragon_Movement_Grounded);
    }
}

float USFDragonMovementComponent::GetMaxSpeed() const
{
    if (IsFlying())
    {
       return DragonFlySpeed;
    }
    
    if (bIsSprinting)
    {
       return DragonRunSpeed;
    }
    
    return DragonWalkSpeed;
}

void USFDragonMovementComponent::SetSprinting(bool bNewSprinting)
{
    bIsSprinting = bNewSprinting;
}

void USFDragonMovementComponent::SetFlyingMode(bool bFly)
{
    UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());

    if (bFly)
    {
       SetMovementMode(MOVE_Flying);
        

       if (ASC)
       {
           ASC->RemoveLooseGameplayTag(SFGameplayTags::Dragon_Movement_Grounded);
           ASC->AddLooseGameplayTag(SFGameplayTags::Dragon_Movement_Flying);
       }
    }
    else
    {
       
       if (OwnerChar && !OwnerChar->GetCharacterMovement()->IsMovingOnGround())
       {
       
            FHitResult Hit;
            FVector Start = OwnerChar->GetActorLocation();
            FVector End = Start - FVector(0, 0, 150.0f); // 발 밑 체크
            FCollisionQueryParams Params;
            Params.AddIgnoredActor(OwnerChar);

            bool bHitGround = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params);
            
            if (bHitGround)
            {
                SetMovementMode(MOVE_Walking);
            }
            else
            {
                SetMovementMode(MOVE_Falling);
            }
       }
       else
       {
           SetMovementMode(MOVE_Walking);
       }
        

       if (ASC)
       {
           ASC->RemoveLooseGameplayTag(SFGameplayTags::Dragon_Movement_Flying);
           ASC->AddLooseGameplayTag(SFGameplayTags::Dragon_Movement_Grounded);
       }
    }
}

void USFDragonMovementComponent::InternalDisableMovement()
{
    StopMovementImmediately();
    if (IsFlying())
    {
        SetFlyingMode(false); 
    }
}