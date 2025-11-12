// Copyright 1998-2024 Epic Games, Inc. All Rights Reserved.

#include "Weapons/SF_WeaponSkillBase.h"
#include "Weapons/SF_DamageDataAsset.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "Kismet/KismetSystemLibrary.h"


UWeaponSkillBase::UWeaponSkillBase()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UWeaponSkillBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 1. 몽타주 확인
	if (!MontageToPlay)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] ActivateAbility: MontageToPlay is not set!"), *GetName());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// [추가됨] 데미지 데이터 에셋 확인
	if (!DamageDataAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] ActivateAbility: DamageDataAsset is not set!"), *GetName());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 2. 어빌리티 커밋 확인 (자원 소모)
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] ActivateAbility: Failed to commit ability (e.g., not enough stamina)."), *GetName());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 3. 몽타주 재생 태스크 생성
	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, 
		FName("PlayMontageTask"), 
		MontageToPlay, 
		1.0f, NAME_None, true, 1.0f, 0.0f, false
	);

	if (MontageTask)
	{
		// 몽타주 이벤트 바인딩
		MontageTask->OnCompleted.AddDynamic(this, &UWeaponSkillBase::OnMontageCompleted);
		MontageTask->OnInterrupted.AddDynamic(this, &UWeaponSkillBase::OnMontageInterrupted);
		MontageTask->OnCancelled.AddDynamic(this, &UWeaponSkillBase::OnMontageCancelled);

		// 태스크 활성화
		MontageTask->Activate();

		// 4. 데미지 이벤트 대기 태스크 생성 (데이터 에셋에서 태그 가져옴)
		DamageEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			DamageDataAsset->DamageEventTag,
			nullptr,
			true,
			false
		);

		if (DamageEventTask)
		{
			// 데미지 이벤트가 수신되면, OnDamageEventReceived 함수를 호출하도록 바인딩
			DamageEventTask->EventReceived.AddDynamic(this, &UWeaponSkillBase::OnDamageEventReceived);
			DamageEventTask->Activate();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] ActivateAbility: Failed to create DamageEventTask."), *GetName());
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] ActivateAbility: Failed to create MontageTask."), *GetName());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UWeaponSkillBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 데미지 태스크 정리
	if (DamageEventTask)
	{
		DamageEventTask->EndTask();
		DamageEventTask = nullptr;
	}
	
	// 몽타주 태스크 정리
	if (MontageTask)
	{
		MontageTask->EndTask();
		MontageTask = nullptr;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UWeaponSkillBase::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UWeaponSkillBase::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UWeaponSkillBase::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UWeaponSkillBase::OnDamageEventReceived(FGameplayEventData Payload)
{
	// 0. 검증: 데미지 데이터 에셋이 설정되었는지?
	if (!DamageDataAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] OnDamageEventReceived: DamageDataAsset is not set!"), *GetName());
		return;
	}

	if (!DamageDataAsset->DamageEffectClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] OnDamageEventReceived: DamageEffectClass in DamageDataAsset is not set!"), *GetName());
		return;
	}

	ACharacter* MyCharacter = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!MyCharacter) return;

	// 1. 트레이스 설정 (데이터 에셋에서 값을 가져옴)
	FVector Start = MyCharacter->GetActorLocation();
	FVector End = Start + (MyCharacter->GetActorForwardVector() * DamageDataAsset->TraceDistance);
	float Radius = DamageDataAsset->TraceRadius;
	
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(MyCharacter);

	TArray<FHitResult> HitResults;
	
	// 2. 스피어 트레이스 실행
	UKismetSystemLibrary::SphereTraceMulti(
		this,
		Start,
		End,
		Radius,
		UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Pawn),
		false,
		ActorsToIgnore,
		EDrawDebugTrace::ForDuration,
		HitResults,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		DamageDataAsset->TraceVisualizationDuration
	);

	// 3. 맞은 대상들에게 데미지 GE 적용
	TSet<AActor*> HitActors;

	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor && !HitActors.Contains(HitActor))
		{
			HitActors.Add(HitActor);
			
			IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(HitActor);
			if (ASCInterface)
			{
				UAbilitySystemComponent* TargetASC = ASCInterface->GetAbilitySystemComponent();
				if (TargetASC)
				{
					// 크리티컬 여부 판정
					bool bIsCritical = DamageDataAsset->IsHitCritical();
					float FinalDamage = DamageDataAsset->GetFinalDamage(bIsCritical);

					// GE 스펙 생성
					FGameplayEffectSpecHandle DamageSpecHandle = MakeOutgoingGameplayEffectSpec(DamageDataAsset->DamageEffectClass);
					
					// SetByCaller 태그를 데이터 에셋에서 가져온 값으로 설정
					DamageSpecHandle.Data->SetSetByCallerMagnitude(DamageDataAsset->SetByCallerTag, FinalDamage);

					// 크리티컬 태그 추가 (옵션)
					if (bIsCritical)
					{
						DamageSpecHandle.Data->AddDynamicAssetTag(FGameplayTag::RequestGameplayTag(FName("Damage.Critical")));
					}

					// GE 적용
					TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data);

					UE_LOG(LogTemp, Log, TEXT("[%s] Applied %.1f damage to %s (Critical: %s)"), 
						*GetName(), FinalDamage, *HitActor->GetName(), bIsCritical ? TEXT("Yes") : TEXT("No"));
				}
			}
		}
	}
}