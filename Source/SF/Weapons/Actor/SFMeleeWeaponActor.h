// SFMeleeWeaponActor.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/SFTraceActorInterface.h"
#include "SFMeleeWeaponActor.generated.h"

UCLASS()
class SF_API ASFMeleeWeaponActor : public AActor, public ISFTraceActorInterface
{
	GENERATED_BODY()

public:
	ASFMeleeWeaponActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// ISFTraceActorInterface
	virtual bool CanBeTraced() const override;
	virtual void OnTraced(const FHitResult& HitInfo, AActor* WeaponOwner) override;
	virtual void OnTraceStart(AActor* WeaponOwner) override;
	virtual void OnTraceEnd(AActor* WeaponOwner) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<class UBoxComponent> WeaponCollision;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Debug")
	bool bShowDebug = false;

private:
	UFUNCTION()
	void OnWeaponOverlap(UPrimitiveComponent* OverlappedComp,  AActor* OtherActor, UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,bool bFromSweep,const FHitResult& SweepResult);

	// [추가] 실제 타격 로직을 처리하는 내부 함수
	void ProcessOverlap(AActor* TargetActor, UPrimitiveComponent* TargetComp);

	UPROPERTY()
	TObjectPtr<AActor> CurrentWeaponOwner;
    
	UPROPERTY()
	TSet<TObjectPtr<AActor>> HitActorsThisAttack;

	// [추가] 연속 공격 처리를 위한 카운터
	int32 TraceRefCounts = 0;
};