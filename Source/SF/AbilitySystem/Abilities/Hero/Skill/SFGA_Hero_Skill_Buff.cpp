#include "SFGA_Hero_Skill_Buff.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Character/SFCharacterBase.h"
#include "AbilitySystem/SFAbilitySystemComponent.h"


USFGA_Hero_Skill_Buff::USFGA_Hero_Skill_Buff(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}



//=========================ActivateAbility=========================
void USFGA_Hero_Skill_Buff::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle,ActorInfo,ActivationInfo,TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle,ActorInfo,ActivationInfo,true,true);
		return;
	}

	if (BuffMontage && ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		auto* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, BuffMontage, 1.f, NAME_None, false, 1.f);

		if(Task)
		{
			Task->OnInterrupted.AddDynamic(this, &USFGA_Hero_Skill_Buff::OnMontageInterrupted);
			Task->OnCancelled.AddDynamic(this, &USFGA_Hero_Skill_Buff::OnMontageInterrupted);
			Task->OnBlendOut.AddDynamic(this, &USFGA_Hero_Skill_Buff::OnMontageBlendOut);

			Task->ReadyForActivation();
		}
	}

	if (StartEventTag.IsValid())
	{
		auto* Wait = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, StartEventTag);
		Wait->EventReceived.AddDynamic(this, &USFGA_Hero_Skill_Buff::OnReceivedSkillEvent);
		Wait->ReadyForActivation();
	}
}
//====================================================================



//=================GameplayEvent (Notify로부터 신호 받음)===============
void USFGA_Hero_Skill_Buff::OnReceivedSkillEvent(FGameplayEventData Payload)
{
	auto* ASC = CurrentActorInfo ? CurrentActorInfo->AbilitySystemComponent.Get() : nullptr;
	if(!ASC) return;

	if (GroundCueTag.IsValid())
		ASC->AddGameplayCue(GroundCueTag);

	//이때 스킬 사용 확정
	bSkillActivatedByEvent = true; 
	OnSkillEventTriggered(); 
}
//====================================================================



//====================BlueprintNativeEvent 기본 구현===================
void USFGA_Hero_Skill_Buff::OnSkillEventTriggered_Implementation()
{
	//자식 Ability에서 Override해서 사용
}
//====================================================================

//====================Montage Interruption============================
void USFGA_Hero_Skill_Buff::OnMontageInterrupted()
{
	if(bSkillActivatedByEvent)
		return; //스킬 사용이 확정되었으면 모션 캔슬되어도 스킬 유지

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
//====================================================================

//=========================Montage BlendOut===========================
void USFGA_Hero_Skill_Buff::OnMontageBlendOut()
{
	if(!IsActive()) return;

	if(bSkillActivatedByEvent)
		return;	//스킬 사용이 확정되었으면 취소 금지

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
//====================================================================

//============================Buff Handling===========================
void USFGA_Hero_Skill_Buff::ApplyAura(AActor* Target)
{
	if(!AuraCueTag.IsValid()) return;
	if(!Target->ActorHasTag("Player")) return;

	auto* Char = Cast<ASFCharacterBase>(Target);
	if(!Char) return;

	auto* ASC = Cast<USFAbilitySystemComponent>(Char->GetAbilitySystemComponent());
	if(!ASC) return;

	auto Ctx = ASC->MakeEffectContext();
	Ctx.AddSourceObject(this);

	auto Spec = ASC->MakeOutgoingSpec(BuffEffectClass, BuffLevel, Ctx);
	if(!Spec.IsValid()) return;

	auto Handle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	if(!Handle.IsValid()) return;

	ASC->AddGameplayCue(AuraCueTag);
	ActiveAuraEffects.Add(Target, Handle);
}

void USFGA_Hero_Skill_Buff::RemoveAura(AActor* Target)
{
	if(!ActiveAuraEffects.Contains(Target)) return;

	auto* Char = Cast<ASFCharacterBase>(Target);
	if(!Char) return;

	auto* ASC = Cast<USFAbilitySystemComponent>(Char->GetAbilitySystemComponent());
	if(ASC)
	{
		auto Handle = ActiveAuraEffects[Target];
		if(Handle.IsValid()) ASC->RemoveActiveGameplayEffect(Handle);
		if(AuraCueTag.IsValid()) ASC->RemoveGameplayCue(AuraCueTag);
	}

	ActiveAuraEffects.Remove(Target);
}
//=====================================================================

//=============================EndAbility==============================
void USFGA_Hero_Skill_Buff::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	bSkillActivatedByEvent = false;
	
	auto* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;

	if(ASC && GroundCueTag.IsValid())
		ASC->RemoveGameplayCue(GroundCueTag);

	TArray<AActor*> Keys;
	ActiveAuraEffects.GetKeys(Keys);
	for(auto* T : Keys)
		RemoveAura(T);

	ActiveAuraEffects.Empty();

	Super::EndAbility(Handle,ActorInfo,ActivationInfo,bReplicateEndAbility,bWasCancelled);
}
//=======================================================================