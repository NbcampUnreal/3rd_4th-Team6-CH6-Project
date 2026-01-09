#include "SFGC_WaveEffect.h"

#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "AbilitySystem/GameplayCues/Data/SFDA_WaveEffectData.h"
#include "Kismet/GameplayStatics.h"

bool USFGC_WaveEffect::OnExecute_Implementation(AActor* Target, const FGameplayCueParameters& Parameters) const
{
	if (!Target)
	{
		return false;
	}

	UWorld* World = Target->GetWorld();
	if (!World)
	{
		return false;
	}

	// Payload에서 추출, 없으면 기본값 사용
	UNiagaraSystem* NiagaraToSpawn = DefaultNiagaraSystem;
	USoundBase* SoundToPlay = DefaultSound;
	float ScaleMultiplier = 1.0f;

	if (const USFDA_WaveEffectData* EffectData = Cast<USFDA_WaveEffectData>(Parameters.SourceObject.Get()))
	{
		if (EffectData->NiagaraSystem)
		{
			NiagaraToSpawn = EffectData->NiagaraSystem;
		}
		if (EffectData->Sound)
		{
			SoundToPlay = EffectData->Sound;
		}
		ScaleMultiplier = EffectData->ScaleMultiplier;
	}

	// 스폰 위치
	FVector SpawnLocation = FVector(Parameters.Location);
	if (SpawnLocation.IsNearlyZero())
	{
		SpawnLocation = Target->GetActorLocation();
	}

	// 바닥 보정
	if (bSnapToFloor)
	{
		FHitResult HitResult;
		FVector TraceStart = SpawnLocation + FVector(0.f, 0.f, 50.f);
		FVector TraceEnd = SpawnLocation - FVector(0.f, 0.f, 200.f);

		if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility))
		{
			SpawnLocation = HitResult.ImpactPoint;
		}
	}

	// 스케일 계산: (판정범위 / 에셋기준반경) * 보정계수
	FVector FinalScale = FVector(1.f);
	if (Parameters.RawMagnitude > 0.f)
	{
		// 기본 가정: Niagara 에셋이 스케일 1에서 반경 100
		constexpr float AssumedBaseRadius = 100.f;
		float Scale = (Parameters.RawMagnitude / AssumedBaseRadius) * ScaleMultiplier;
		FinalScale = FVector(Scale);
	}


	// 나이아가라 스폰
	if (NiagaraToSpawn)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			NiagaraToSpawn,
			SpawnLocation,
			FRotator::ZeroRotator,
			FinalScale,
			true,
			true
		);
	}

	// 사운드 재생
	if (SoundToPlay)
	{
		UGameplayStatics::PlaySoundAtLocation(
			World,
			SoundToPlay,
			SpawnLocation,
			VolumeMultiplier,
			PitchMultiplier
		);
	}

	return true;
}