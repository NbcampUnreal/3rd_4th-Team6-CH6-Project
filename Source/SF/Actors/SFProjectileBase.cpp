#include "SFProjectileBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h" // [추가] 헤더 포함
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraComponent.h"
#include "Character/SFCharacterBase.h"

ASFProjectileBase::ASFProjectileBase()
{
    PrimaryActorTick.bCanEverTick = false;

    // 1. 충돌체 (Root)
    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
    CollisionComponent->InitSphereRadius(15.0f);
    CollisionComponent->SetCollisionProfileName(TEXT("Projectile"));
    SetRootComponent(CollisionComponent);

    // 2. [추가] 스태틱 메시 (Visual)
    ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
    ProjectileMesh->SetupAttachment(CollisionComponent); // 루트(구체)에 부착
    ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 충돌은 구체가 담당하므로 메시는 끔

    // 3. 무브먼트
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->UpdatedComponent = CollisionComponent;
    ProjectileMovement->InitialSpeed = InitialSpeed;
    ProjectileMovement->MaxSpeed = MaxSpeed;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;
    ProjectileMovement->ProjectileGravityScale = GravityScale;

    // 4. 이펙트
    ProjectileEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ProjectileEffect"));
    ProjectileEffect->SetupAttachment(RootComponent);

    InitialLifeSpan = LifeSpan;
}

void ASFProjectileBase::SetOwner(AActor* NewOwner)
{
    Super::SetOwner(NewOwner);

    ASFCharacterBase* SFCharacter = Cast<ASFCharacterBase>(NewOwner);
    if (SFCharacter)
    {
       OwnerChar = SFCharacter;
       if (CollisionComponent && SFCharacter)
       {
          CollisionComponent->IgnoreActorWhenMoving(OwnerChar, true);
       }
    }
}

void ASFProjectileBase::BeginPlay()
{
    Super::BeginPlay();

    CollisionComponent->OnComponentHit.AddDynamic(this, &ASFProjectileBase::OnProjectileHit);
}

void ASFProjectileBase::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    Destroy();
}