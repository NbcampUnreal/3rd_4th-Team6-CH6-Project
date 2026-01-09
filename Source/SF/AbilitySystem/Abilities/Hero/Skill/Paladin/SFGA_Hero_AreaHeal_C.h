#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Hero/Skill/SFGA_Skill_Melee.h"
#include "GameplayTagContainer.h"
#include "SFGA_Hero_AreaHeal_C.generated.h"

class UGameplayEffect;
class UAnimMontage;
class UNiagaraSystem;
class UNiagaraComponent;
class USkeletalMeshComponent;

UCLASS()
class SF_API USFGA_Hero_AreaHeal_C : public USFGA_Skill_Melee
{
	GENERATED_BODY()

public:
	USFGA_Hero_AreaHeal_C(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	//=====================Ability Lifecycle============================
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;
	//===================================================================

	//=====================이벤트 수신 (번개 타격 타이밍)===============
	UFUNCTION()
	void OnLightningImpact(FGameplayEventData Payload);
	//===================================================================

	UFUNCTION()
	void OnMontageEnded();

	//=====================현재 장착 무기 메쉬 찾기=======================
	USkeletalMeshComponent* FindCurrentWeaponMesh(class ASFCharacterBase* OwnerChar) const;
	//===================================================================

protected:

	//=====================Gameplay Tags===============================
	UPROPERTY(EditAnywhere, Category="SF|Tags")
	FGameplayTag LightningCueTag; //FX 호출용 GC 태그

	UPROPERTY(EditAnywhere, Category="SF|Tags")
	FGameplayTag LightningEventTag;	//실제 로직 수행 AnimNotify GameplayEvent 태그
	//==================================================================

	UPROPERTY(EditAnywhere, Category="SF|Gameplay")
	float StrikeDistance = 300.f; //공격 소환 거리

	UPROPERTY(EditAnywhere, Category="SF|Gameplay")
	float StrikeRadius = 200.f; //공격 범위

	UPROPERTY(EditAnywhere, Category="SF|Gameplay")
	TSubclassOf<UGameplayEffect> DebuffGE; //디버프 GE 적용
	//==================================================================

	//=====================Animation===============================
	UPROPERTY(EditAnywhere, Category="SF|Animation")
	UAnimMontage* LightningMontage;
	//==================================================================

	//=====================Trail FX===============================
	UPROPERTY(EditDefaultsOnly, Category="SF|VFX")
	UNiagaraSystem* SwordTrailFX;

	UPROPERTY()
	UNiagaraComponent* TrailComp;

	FTimerHandle TrailUpdateHandle;
	FTimerHandle TrailFadeHandle;
	//==================================================================
};
