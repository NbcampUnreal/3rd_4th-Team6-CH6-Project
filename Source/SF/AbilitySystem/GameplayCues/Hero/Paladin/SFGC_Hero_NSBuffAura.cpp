#include "SFGC_Hero_NSBuffAura.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Character.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SFGC_Hero_NSBuffAura)

//=====================생성자=====================
ASFGC_Hero_NSBuffAura::ASFGC_Hero_NSBuffAura()
{
	PrimaryActorTick.bCanEverTick = false; //Tick 없음
	bAutoDestroyOnRemove = true; //OnRemove 후 자동 정리

	AuraComponent = nullptr;
	AuraAudioComp = nullptr;
}
//================================================


//=====================OnActive=====================
bool ASFGC_Hero_NSBuffAura::OnActive_Implementation(
	AActor* Target,
	const FGameplayCueParameters& Parameters)
{
	if(!Target) return false;

	//이미 Aura가 있다면 중복 생성 방지
	if(AuraComponent && AuraComponent->IsActive()) return false;

	USceneComponent* AttachComp = nullptr;
	if(ACharacter* Char = Cast<ACharacter>(Target)) AttachComp = Char->GetMesh();
	else AttachComp = Target->GetRootComponent();

	//=====================Niagara FX=====================
	if(AuraNiagaraFX)
	{
		AuraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			AuraNiagaraFX,
			AttachComp,
			NAME_None,
			AuraLocationOffset,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			false
		);
	}
	//================================================

	//=====================Loop Sound=====================
	if(AuraLoopSound)
	{
		AuraAudioComp = UGameplayStatics::SpawnSoundAttached(
			AuraLoopSound,
			AttachComp
		);
		AuraAudioComp->bAutoDestroy = false; //OnRemove에서 직접 정리
	}
	//================================================

	return true;
}
//================================================


//=====================WhileActive=====================
bool ASFGC_Hero_NSBuffAura::WhileActive_Implementation(
	AActor* Target,
	const FGameplayCueParameters& Parameters)
{
	return true;
}
//================================================


//=====================OnRemove=====================
bool ASFGC_Hero_NSBuffAura::OnRemove_Implementation(
	AActor* Target,
	const FGameplayCueParameters& Parameters)
{
	if(AuraComponent)
	{
		AuraComponent->Deactivate();
		AuraComponent->DestroyComponent();
		AuraComponent = nullptr;
	}

	if(AuraAudioComp)
	{
		AuraAudioComp->Stop();
		AuraAudioComp->DestroyComponent();
		AuraAudioComp = nullptr;
	}

	return true;
}
//================================================
