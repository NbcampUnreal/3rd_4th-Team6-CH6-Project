#include "SFDragonFireballProjectile.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GenericTeamAgentInterface.h"
#include "AbilitySystem/GameplayEvent/SFGameplayEventTags.h"
#include "Character/SFCharacterBase.h"

ASFDragonFireballProjectile::ASFDragonFireballProjectile()
{
	InitialSpeed = 1500.f;
	MaxSpeed = 2000.f;
	GravityScale = 0.5f;
}

void ASFDragonFireballProjectile::OnProjectileHit( UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    
    if (!OtherActor || OtherActor == this || OtherActor == Owner)
    {
        return;
    }

    if (!OwnerChar)
    {
        Destroy();
        return;
    }
    
    bool bIsHostile = false;

    if (OtherActor->Implements<UGenericTeamAgentInterface>())
    {
        const ETeamAttitude::Type Attitude = OwnerChar->GetTeamAttitudeTowards(*OtherActor);

        bIsHostile = (Attitude == ETeamAttitude::Hostile);
    }
    
    if (ExplosionEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            ExplosionEffect,
            Hit.ImpactPoint,
            Hit.ImpactNormal.Rotation()
        );
    }
    
    if (bIsHostile && DamageEffectClass)
    {
        UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);

        UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerChar);

        if (TargetASC && SourceASC)
        {
            FGameplayEffectContextHandle ContextHandle =
                SourceASC->MakeEffectContext();

            ContextHandle.AddHitResult(Hit);
            ContextHandle.AddInstigator(OwnerChar, OwnerChar);
            ContextHandle.AddSourceObject(this);

            FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec( DamageEffectClass,1.0f,ContextHandle);

            if (SpecHandle.IsValid())
            {
                
                SpecHandle.Data.Get()->SetSetByCallerMagnitude(
                    SFGameplayTags::Data_Damage_BaseDamage,
                    BaseDamage
                );

                SourceASC->ApplyGameplayEffectSpecToTarget(
                    *SpecHandle.Data.Get(),
                    TargetASC
                );
            }
        }
    }
    
    Destroy();
}

