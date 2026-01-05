#pragma once

#include "CoreMinimal.h"
#include "SFProjectileBase.h"
#include "SFDragonFireballProjectile.generated.h"

class UNiagaraSystem;

UCLASS()
class SF_API ASFDragonFireballProjectile : public ASFProjectileBase
{
	GENERATED_BODY()

public:
	ASFDragonFireballProjectile();

protected:
	virtual void OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float BaseDamage = 50.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float ExplosionRadius = 200.0f;

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	TObjectPtr<UNiagaraSystem> ExplosionEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Damge")
	TSubclassOf<class UGameplayEffect> DamageEffectClass;
};

