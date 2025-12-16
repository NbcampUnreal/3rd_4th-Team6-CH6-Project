#include "SFAnimNotify_WeaponNiagaraEffect.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Character/SFCharacterBase.h"
#include "Equipment/EquipmentComponent/SFEquipmentComponent.h"
#include "Weapons/Actor/SFEquipmentBase.h"

USFAnimNotify_WeaponNiagaraEffect::USFAnimNotify_WeaponNiagaraEffect()
{
#if WITH_EDITORONLY_DATA
	bShouldFireInEditor = false;
#endif
}

void USFAnimNotify_WeaponNiagaraEffect::Notify(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	USkeletalMeshComponent* WeaponMeshComponent = GetWeaponMeshComponent(MeshComponent);
	Super::Notify(WeaponMeshComponent, Animation, EventReference);
}

UFXSystemComponent* USFAnimNotify_WeaponNiagaraEffect::SpawnEffect(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	UFXSystemComponent* ReturnComp = nullptr;

	if (Template)
	{
		if (Template->IsLooping())
		{
			return ReturnComp;
		}

		if (Attached)
		{
			ReturnComp = UNiagaraFunctionLibrary::SpawnSystemAttached(Template, MeshComp, SocketName, LocationOffset, RotationOffset, AttachLocationType, true);
		}
		else
		{
			const FTransform MeshTransform = MeshComp->GetSocketTransform(SocketName);
			ReturnComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(MeshComp->GetWorld(), Template, MeshTransform.TransformPosition(LocationOffset), (MeshTransform.GetRotation() * RotationOffsetQuat).Rotator(), FVector(1.0f),true);
		}

		if (ReturnComp != nullptr)
		{
			ReturnComp->SetUsingAbsoluteScale(bAbsoluteScale);
			ReturnComp->SetRelativeScale3D_Direct(Scale);
		}
	}

	return ReturnComp;
}

USkeletalMeshComponent* USFAnimNotify_WeaponNiagaraEffect::GetWeaponMeshComponent(USkeletalMeshComponent* CharacterMeshComponent) const
{
	USkeletalMeshComponent* WeaponMeshComponent = nullptr;

	if (ASFCharacterBase* Character = Cast<ASFCharacterBase>(CharacterMeshComponent->GetOwner()))
	{
		if (USFEquipmentComponent* EquipmentComponent = Character->FindComponentByClass<USFEquipmentComponent>())
		{
			if (ASFEquipmentBase* WeaponActor = Cast<ASFEquipmentBase>(EquipmentComponent->GetFirstEquippedActorBySlot(EquipmentSlotTag)))
			{
				WeaponMeshComponent = WeaponActor->GetMeshComponent();
			}
		}
	}
	return WeaponMeshComponent;
}
