#include "SFGCN_BuffAura.h"

#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "AbilitySystem/GameplayCues/Data/SFDA_BuffAuraEffectData.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

ASFGCN_BuffAura::ASFGCN_BuffAura()
{
	bAutoDestroyOnRemove = true;
}

bool ASFGCN_BuffAura::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	if (!MyTarget)
	{
		return false;
	}

	// 중복 생성 방지
	if (SpawnedNiagaraComponent && SpawnedNiagaraComponent->IsActive())
	{
		return false;
	}

	// Payload에서 추출, 없으면 기본값 사용
	UNiagaraSystem* NiagaraToSpawn = DefaultNiagaraSystem;
	USoundBase* SoundToPlay = DefaultLoopSound;

	if (Parameters.EffectContext.IsValid())
	{
		if (const USFDA_BuffAuraEffectData* EffectData = Cast<USFDA_BuffAuraEffectData>(Parameters.EffectContext.GetSourceObject()))
		{
			if (EffectData->NiagaraSystem)
			{
				NiagaraToSpawn = EffectData->NiagaraSystem;
			}
			if (EffectData->LoopSound)
			{
				SoundToPlay = EffectData->LoopSound;
			}
		}
	}

	// 부착 컴포넌트 결정
	USceneComponent* AttachComponent = nullptr;
	if (ACharacter* Character = Cast<ACharacter>(MyTarget))
	{
		AttachComponent = Character->GetMesh();
	}
	else
	{
		AttachComponent = MyTarget->GetRootComponent();
	}

	if (!AttachComponent)
	{
		return false;
	}

	// 나이아가라 FX 부착
	if (NiagaraToSpawn)
	{
		SpawnedNiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			NiagaraToSpawn,
			AttachComponent,
			AttachSocketName,
			LocationOffset,
			FRotator::ZeroRotator,
			FXScale,
			EAttachLocation::KeepRelativeOffset,
			true,
			ENCPoolMethod::None,
			true
		);
	}

	// 루프 사운드 부착
	if (SoundToPlay)
	{
		APawn* TargetPawn = Cast<APawn>(MyTarget);
		if (TargetPawn && TargetPawn->IsLocallyControlled())
		{
			SpawnedAudioComponent = UGameplayStatics::SpawnSoundAttached(
				SoundToPlay,
				AttachComponent,
				AttachSocketName
			);

			if (SpawnedAudioComponent)
			{
				SpawnedAudioComponent->bAutoDestroy = false;
			}
		}
	}

	return true;
}

bool ASFGCN_BuffAura::WhileActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	return true;
}

bool ASFGCN_BuffAura::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	if (SpawnedNiagaraComponent)
	{
		SpawnedNiagaraComponent->Deactivate();
		SpawnedNiagaraComponent->DestroyComponent();
		SpawnedNiagaraComponent = nullptr;
	}

	if (SpawnedAudioComponent)
	{
		SpawnedAudioComponent->Stop();
		SpawnedAudioComponent->DestroyComponent();
		SpawnedAudioComponent = nullptr;
	}

	return true;
}
