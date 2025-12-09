#include "SFGC_Hero_AreaHeal_C_Lightning.h"

#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundBase.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SFGC_Hero_AreaHeal_C_Lightning)

//=====================스폰 위치 계산=====================
FVector USFGC_Hero_AreaHeal_C_Lightning::ResolveSpawnLocation(
	AActor* Target,
	const FGameplayCueParameters& Parameters) const
{
	if (!Parameters.Location.IsNearlyZero()) return Parameters.Location; //GameplayCue에서 Location 직접 전달된 경우

	if (bUseTargetFloorIfNoLocation && Target) //없으면 발밑 라인추적
	{
		UWorld* World = Target->GetWorld();
		if (!World) return Target->GetActorLocation();

		FHitResult Hit;
		FVector Start = Target->GetActorLocation() + FVector(0,0,FloorTraceUp);
		FVector End   = Target->GetActorLocation() - FVector(0,0,FloorTraceDown);

		FCollisionQueryParams Params(SCENE_QUERY_STAT(SFGC_AreaHeal_C_FloorTrace),false,Target);

		if (World->LineTraceSingleByChannel(Hit,Start,End,ECC_Visibility,Params))
			return Hit.ImpactPoint + FVector(0,0,5.f); //지면 약간 위

		return Target->GetActorLocation(); //Trace 실패
	}

	return Target ? Target->GetActorLocation() : FVector::ZeroVector; //fallback
}
//============================================================


//=====================GameplayCue 실행=====================
void USFGC_Hero_AreaHeal_C_Lightning::HandleGameplayCue(
	AActor* Target,
	EGameplayCueEvent::Type EventType,
	const FGameplayCueParameters& Parameters)
{
	if (EventType!=EGameplayCueEvent::Executed && EventType!=EGameplayCueEvent::OnActive) return; //원샷 전용
	if (!Target) return;

	UWorld* World = Target->GetWorld();
	if (!World) return;

	const FVector SpawnLocation = ResolveSpawnLocation(Target,Parameters);
	const FRotator SpawnRotation = FRotator::ZeroRotator;

	//=====================파티클1=====================
	if (LightningEffect1)
	{
		FTransform T(SpawnRotation,SpawnLocation,ParticleScale);
		UGameplayStatics::SpawnEmitterAtLocation(World,LightningEffect1,T,true); //스폰
	}
	//================================================

	//=====================파티클2=====================
	if (LightningEffect2)
	{
		FTransform T(SpawnRotation,SpawnLocation,ParticleScale);
		UGameplayStatics::SpawnEmitterAtLocation(World,LightningEffect2,T,true); //추가 스폰
	}
	//================================================

	//=====================사운드1=====================
	if (LightningSound1)
	{
		UGameplayStatics::PlaySoundAtLocation(
			World,LightningSound1,SpawnLocation,VolumeMultiplier,PitchMultiplier); //재생
	}
	//================================================

	//=====================사운드2=====================
	if (LightningSound2)
	{
		UGameplayStatics::PlaySoundAtLocation(
			World,LightningSound2,SpawnLocation,VolumeMultiplier,PitchMultiplier); //추가 재생
	}
	//================================================
}
//============================================================
