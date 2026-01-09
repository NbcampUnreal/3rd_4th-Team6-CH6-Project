// Fill out your copyright notice in the Description page of Project Settings.


#include "SFAnimNotifyState_OverlayEffect.h"

#include "Character/SFCharacterBase.h"
#include "Curves/CurveLinearColor.h"
#include "Equipment/EquipmentComponent/SFEquipmentComponent.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Weapons/Actor/SFEquipmentBase.h"
#include "Weapons/Actor/SFMeleeWeaponActor.h"

USFAnimNotifyState_OverlayEffect::USFAnimNotifyState_OverlayEffect(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	bShouldFireInEditor = false;
#endif
}

void USFAnimNotifyState_OverlayEffect::NotifyBegin(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComponent, Animation, TotalDuration, EventReference);

	if (OverlayTargetType == ESFOverlayTargetType::None)
	{
		return;
	}
	
	FSFOverlayEffectProgressInfo& NewProgressInfo = ProgressInfoMap.Add(MeshComponent);
	NewProgressInfo.OverlayMaterialInstance = UKismetMaterialLibrary::CreateDynamicMaterialInstance(MeshComponent, OverlayMaterial);
	
	switch (OverlayTargetType)
	{
	case ESFOverlayTargetType::Equipment:
		ApplyEquipmentMeshComponent(NewProgressInfo, MeshComponent);
		break;
                                      		
	case ESFOverlayTargetType::Character:
		ApplyCharacterMeshComponents(NewProgressInfo, MeshComponent);
		break;
                                      		
	case ESFOverlayTargetType::All:
		ApplyAllEquipmentMeshComponents(NewProgressInfo, MeshComponent);
		ApplyCharacterMeshComponents(NewProgressInfo, MeshComponent);
		break;
	}
}

void USFAnimNotifyState_OverlayEffect::NotifyTick(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComponent, Animation, FrameDeltaTime, EventReference);

	if (FSFOverlayEffectProgressInfo* ProgressInfo = ProgressInfoMap.Find(MeshComponent))
	{
		const float RateScale = bApplyRateScaleToProgress ? Animation->RateScale : 1.0f;
		ProgressInfo->ElapsedTime += FrameDeltaTime * RateScale;

		if (UMaterialInstanceDynamic* OverlayMaterialInstance = ProgressInfo->OverlayMaterialInstance)
		{
			const FLinearColor& Value = LinearColorCurve->GetLinearColorValue(ProgressInfo->ElapsedTime);
			OverlayMaterialInstance->SetVectorParameterValue(ParameterName, Value);
			OverlayMaterialInstance->SetScalarParameterValue(ParameterAlpha, Value.A);
		}
	}
}

void USFAnimNotifyState_OverlayEffect::NotifyEnd(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (FSFOverlayEffectProgressInfo* ProgressInfo = ProgressInfoMap.Find(MeshComponent))
	{
		for (TWeakObjectPtr<UMeshComponent> CachedMeshComponent : ProgressInfo->MeshComponents)
		{
			if (CachedMeshComponent.IsValid())
			{
				CachedMeshComponent->SetOverlayMaterial(nullptr);
			}
		}
	}
	
	ProgressInfoMap.Remove(MeshComponent);
	
	Super::NotifyEnd(MeshComponent, Animation, EventReference);
}

void USFAnimNotifyState_OverlayEffect::ApplyEquipmentMeshComponent(FSFOverlayEffectProgressInfo& ProgressInfo, USkeletalMeshComponent* MeshComponent)
{
	if (EquipmentSlotTag == FGameplayTag::EmptyTag)
	{
		return;
	}
	
	if (ASFCharacterBase* SFCharacter = Cast<ASFCharacterBase>(MeshComponent->GetOwner()))
	{
		if (USFEquipmentComponent* EquipManager = SFCharacter->FindComponentByClass<USFEquipmentComponent>())
		{
			AActor* EquipmentActor = EquipManager->GetFirstEquippedActorBySlot(EquipmentSlotTag);
			if (ASFEquipmentBase* EquipmentBaseActor = Cast<ASFEquipmentBase>(EquipmentActor))
			{
				USkeletalMeshComponent* EquipmentMeshComponent = EquipmentBaseActor->MeshComponent;
				EquipmentMeshComponent->SetOverlayMaterial(ProgressInfo.OverlayMaterialInstance);
				ProgressInfo.MeshComponents.Add(EquipmentMeshComponent);
			}
			else if (ASFMeleeWeaponActor* MeleeWeaponActor = Cast<ASFMeleeWeaponActor>(EquipmentActor))
			{
				// TODO : ASFMeleeWeaponActor를 SkeletalMeshComponent로 전환 필요
			}
		}
	}
}

void USFAnimNotifyState_OverlayEffect::ApplyAllEquipmentMeshComponents(FSFOverlayEffectProgressInfo& ProgressInfo, USkeletalMeshComponent* MeshComponent)
{
	if (ASFCharacterBase* SFCharacter = Cast<ASFCharacterBase>(MeshComponent->GetOwner()))
	{
		if (USFEquipmentComponent* EquipmentComponent = SFCharacter->FindComponentByClass<USFEquipmentComponent>())
		{
			TArray<AActor*> EquippedActors;
			EquipmentComponent->GetAllEquippedActors(EquippedActors);

			for (AActor* EquippedActor : EquippedActors)
			{
				if (ASFEquipmentBase* EquipmentBaseActor = Cast<ASFEquipmentBase>(EquippedActor))
				{
					USkeletalMeshComponent* WeaponMeshComponent = EquipmentBaseActor->MeshComponent;
					WeaponMeshComponent->SetOverlayMaterial(ProgressInfo.OverlayMaterialInstance);
					ProgressInfo.MeshComponents.Add(WeaponMeshComponent);
				}
				else if (ASFMeleeWeaponActor* MeleeWeaponActor = Cast<ASFMeleeWeaponActor>(EquippedActor))
				{
					// TODO : ASFMeleeWeaponActor의 SkeletalMeshComponent 전환 필요
				}
			}
		}
	}
}

void USFAnimNotifyState_OverlayEffect::ApplyCharacterMeshComponents(FSFOverlayEffectProgressInfo& ProgressInfo, USkeletalMeshComponent* MeshComponent)
{
	if (ASFCharacterBase* SFCharacter = Cast<ASFCharacterBase>(MeshComponent->GetOwner()))
	{
		TArray<UMeshComponent*> CharacterMeshComponents;
		SFCharacter->GetMeshComponents(CharacterMeshComponents);

		for (UMeshComponent* CharacterMeshComponent : CharacterMeshComponents)
		{
			CharacterMeshComponent->SetOverlayMaterial(ProgressInfo.OverlayMaterialInstance);
			ProgressInfo.MeshComponents.Add(CharacterMeshComponent);
		}
	}
}
