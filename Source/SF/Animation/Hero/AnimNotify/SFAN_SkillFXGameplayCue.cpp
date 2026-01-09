#include "SFAN_SkillFXGameplayCue.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayCueManager.h"
#include "GameFramework/Actor.h"

#include "AbilitySystem/GameplayCues/Hero/SFSkillFXTypes.h"

//================== Notify 구현 ==================
// - Owner 캐릭터를 찾고
// - GameplayCueParameters에 FXData + Phase를 담아서
//   GameplayCue를 실행한다.
//=================================================
void USFAN_SkillFXGameplayCue::Notify(USkeletalMeshComponent* MeshComp,
									  UAnimSequenceBase* Animation,
									  const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	//유효한 Cue 태그가 없으면 실행하지 않음
	if (!GameplayCueTag.IsValid())
	{
		return;
	}

	//FX 데이터가 없으면 이 시스템에서는 의미 없다고 보고 반환
	if (!FXData)
	{
		return;
	}

	//GameplayCue 파라미터 생성
	FGameplayCueParameters Params;

	//SourceObject에 FX DataAsset을 담아서 Cue로 전달
	Params.SourceObject = FXData;

	//Phase 정보를 float로 넘겨서 Cue 쪽에서 다시 enum으로 변환해서 사용
	Params.RawMagnitude = static_cast<float>(Phase);

	//AbilitySystem을 통해 GameplayCue 실행
	//OwnerActor가 IAbilitySystemInterface를 구현하고 있다고 가정
	//또는 GetAbilitySystemComponent를 사용하는 헬퍼가 있을 경우 거기 맞춰 수정
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor);

	if (ASC)
	{
		// ASC가 있으면 정석적인 경로로 실행
		ASC->ExecuteGameplayCue(GameplayCueTag, Params);
	}
	else
	{
		// ASC가 없는 액터라도, 필요하다면 CueManager를 통해 직접 실행
		UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(
			OwnerActor,
			GameplayCueTag,
			EGameplayCueEvent::Executed,
			Params
		);
	}
}
