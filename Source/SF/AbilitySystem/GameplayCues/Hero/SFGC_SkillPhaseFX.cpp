#include "SFGC_SkillPhaseFX.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "SFSkillFXTypes.h"
#include "AbilitySystem/GamePlayCues/Hero/SFSkillFXTypes.h"

//=====================바닥 위치 계산=====================
FVector USFGC_SkillPhaseFX::GetFloorLocationForActor(AActor* Target) const
{
	//타겟 없음
	if (!Target)
	{
		return FVector::ZeroVector;
	}

	UWorld* World = Target->GetWorld();
	if (!World)
	{
		return Target->GetActorLocation();
	}

	const FVector ActorLocation = Target->GetActorLocation();
	const FVector Start = ActorLocation + FVector(0.f, 0.f, 50.f);
	const FVector End   = ActorLocation - FVector(0.f, 0.f, 1000.f);

	FHitResult HitResult;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(SFGC_SkillPhaseFX_FloorTrace), false, Target);

	//바닥 감지
	if (World->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params))
	{
		return HitResult.ImpactPoint;
	}

	//Trace 실패 → Actor 위치 사용
	return ActorLocation;
}
//========================================================


//=====================GameplayCue 처리===================
void USFGC_SkillPhaseFX::HandleGameplayCue(
	AActor* Target,
	EGameplayCueEvent::Type EventType,
	const FGameplayCueParameters& Parameters)
{
	//실행 가능한 이벤트만 처리
	if (EventType != EGameplayCueEvent::Executed &&
	    EventType != EGameplayCueEvent::OnActive &&
	    EventType != EGameplayCueEvent::WhileActive)
	{
		return;
	}

	//타겟 없음
	if (!Target)
	{
		return;
	}

	//FX DataAsset 추출
	const USFDA_SkillPhaseFX* FXData = Cast<USFDA_SkillPhaseFX>(Parameters.SourceObject);
	if (!FXData)
	{
		return;
	}

	//Phase 복원
	const int32 PhaseIndex = FMath::RoundToInt(Parameters.RawMagnitude);
	ESFSkillFXPhase Phase = ESFSkillFXPhase::CastStart;

	switch (PhaseIndex)
	{
	case static_cast<int32>(ESFSkillFXPhase::CastStart):
		Phase = ESFSkillFXPhase::CastStart; break;

	case static_cast<int32>(ESFSkillFXPhase::CastLoop):
		Phase = ESFSkillFXPhase::CastLoop; break;

	default:
		Phase = ESFSkillFXPhase::Activate; break;
	}

	//FX 세트 가져오기
	const FSFSkillPhaseFX& PhaseFX = FXData->GetFXForPhase(Phase);

	UWorld* World = Target->GetWorld();
	if (!World)
	{
		return;
	}

	//스폰 위치(발밑 기준)
	const FVector SpawnLocation = GetFloorLocationForActor(Target);
	const FRotator SpawnRotation = Target->GetActorRotation();

	//=====================Niagara========================
	if (PhaseFX.NiagaraSystem)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			PhaseFX.NiagaraSystem,
			SpawnLocation,
			SpawnRotation,
			PhaseFX.FXScale	//크기 적용
		);
	}
	//====================================================

	//=====================Cascade========================
	if (PhaseFX.CascadeSystem)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			World,
			PhaseFX.CascadeSystem,
			SpawnLocation,
			SpawnRotation,
			PhaseFX.FXScale	//크기 적용
		);
	}
	//====================================================

	//=====================Sound==========================
	if (PhaseFX.Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			World,
			PhaseFX.Sound,
			SpawnLocation,
			PhaseFX.SoundVolume	//볼륨 적용
		);
	}
	//====================================================
}
//========================================================