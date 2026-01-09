#include "SFGC_Hero_AreaHeal_Ground.h"

#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SFGC_Hero_AreaHeal_Ground)

ASFGC_Hero_AreaHeal_Ground::ASFGC_Hero_AreaHeal_Ground()
{
	PrimaryActorTick.bCanEverTick = false; //Tick 미사용
	bAutoDestroyOnRemove = true; //OnRemove 후 자동 Destroy

	SpawnedCascadeComp = nullptr;
	SpawnedNiagaraComp = nullptr;
	SpawnedAudioComp = nullptr;
}


//=====================OnActive=====================
bool ASFGC_Hero_AreaHeal_Ground::OnActive_Implementation(
	AActor* MyTarget,
	const FGameplayCueParameters& Parameters)
{
	if(!MyTarget) return false;
	UWorld* World = MyTarget->GetWorld();
	if(!World) return false;

	//바닥 위치 계산
	FVector SpawnLocation = MyTarget->GetActorLocation(); //기본 = 캐릭터 발 밑
	if(ACharacter* Char = Cast<ACharacter>(MyTarget))
	{
		if(UCapsuleComponent* Capsule = Char->GetCapsuleComponent())
			SpawnLocation.Z -= Capsule->GetScaledCapsuleHalfHeight(); //정확한 지면 위치
	}

	FRotator Rot = FRotator::ZeroRotator;

	//Niagara 우선
	if(GroundNiagara)
	{
		SpawnedNiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			GroundNiagara,
			SpawnLocation,
			Rot,
			FVector(1.f), //크기
			true,
			true
		);
	}
	//Cascade 대체
	else if(GroundCascade)
	{
		SpawnedCascadeComp = UGameplayStatics::SpawnEmitterAtLocation(
			World,
			GroundCascade,
			SpawnLocation,
			Rot,
			FVector(1.f), //크기
			true
		);
	}

	//사운드
	if(GroundSound)
	{
		SpawnedAudioComp = UGameplayStatics::SpawnSoundAtLocation(
			World,
			GroundSound,
			SpawnLocation
		);
	}

	return true;
}
//================================================


//=====================OnRemove=====================
bool ASFGC_Hero_AreaHeal_Ground::OnRemove_Implementation(
	AActor* MyTarget,
	const FGameplayCueParameters& Parameters)
{
	//Niagara 정리
	if(SpawnedNiagaraComp)
	{
		SpawnedNiagaraComp->Deactivate();
		SpawnedNiagaraComp->DestroyComponent();
		SpawnedNiagaraComp = nullptr;
	}

	//Cascade 정리
	if(SpawnedCascadeComp)
	{
		SpawnedCascadeComp->DeactivateSystem();
		SpawnedCascadeComp->DestroyComponent();
		SpawnedCascadeComp = nullptr;
	}

	//Sound 정리
	if(SpawnedAudioComp)
	{
		SpawnedAudioComp->Stop();
		SpawnedAudioComp->DestroyComponent();
		SpawnedAudioComp = nullptr;
	}

	return true;
}
//================================================
