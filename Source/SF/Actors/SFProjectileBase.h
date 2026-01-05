#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SFProjectileBase.generated.h"

class ASFCharacterBase;
class USphereComponent;
class UProjectileMovementComponent;
class UNiagaraComponent;
class UStaticMeshComponent; // [추가] 전방 선언

UCLASS()
class SF_API ASFProjectileBase : public AActor
{
	GENERATED_BODY()

public:
	ASFProjectileBase();

	virtual void SetOwner(AActor* NewOwner) override;
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, 
	   UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionComponent;

	// [추가] 시각적인 메시 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ProjectileMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> ProjectileEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float InitialSpeed = 2000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float MaxSpeed = 3000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float GravityScale = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float LifeSpan = 10.0f;
    
protected:
	UPROPERTY()
	TObjectPtr<ASFCharacterBase> OwnerChar;
    
};