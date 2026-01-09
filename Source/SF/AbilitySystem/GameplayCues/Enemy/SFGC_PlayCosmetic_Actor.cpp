#include "SFGC_PlayCosmetic_Actor.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "AbilitySystem/GameplayCues/Data/SFGameplayCueCosmeticData.h"
#include "Character/SFCharacterBase.h"
#include "Character/SFPawnExtensionComponent.h"
#include "Character/Enemy/SFEnemyData.h"
#include "Character/Enemy/SF_EffectConfig.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"


ASFGC_PlayCosmetic_Actor::ASFGC_PlayCosmetic_Actor()
{
    PrimaryActorTick.bCanEverTick = false; 
    bAutoDestroyOnRemove = true;     
}

bool ASFGC_PlayCosmetic_Actor::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
    
    ASFCharacterBase* Character = Cast<ASFCharacterBase>(MyTarget);
    if (!Character)
    {
        return false;
    }

    USFPawnExtensionComponent* PawnExtComp = USFPawnExtensionComponent::FindPawnExtensionComponent(Character);
    if (!PawnExtComp)
    {
        return false;
    }

    const USFEnemyData* EnemyData = Cast<USFEnemyData>(PawnExtComp->GetPawnData<USFPawnData>());
    if (!EnemyData || !EnemyData->EffectConfig)
    {
        return false;
    }
    
    const FEffectData* EffectData = EnemyData->EffectConfig->EffectDataMap.Find(Parameters.MatchedTagName);
    if (!EffectData)
    {
        return false;
    }
    
    const USFGameplayCueCosmeticData* CosmeticData = Cast<USFGameplayCueCosmeticData>(Parameters.SourceObject);
    
    FName SocketName = NAME_None;
    FVector LocOffset = FVector::ZeroVector;
    FRotator RotOffset = FRotator::ZeroRotator;
    FName ScaleParam = NAME_None;

    if (CosmeticData)
    {
        SocketName = CosmeticData->AttachSocketName;
        LocOffset = CosmeticData->LocationOffset;
        RotOffset = CosmeticData->RotationOffset;
        ScaleParam = CosmeticData->NiagaraScaleParam;
    }

    USkeletalMeshComponent* Mesh = MyTarget->FindComponentByClass<USkeletalMeshComponent>();
    
    if (!EffectData->Effect_NiagaraSystem.IsNull())
    {
        UNiagaraSystem* NS = EffectData->Effect_NiagaraSystem.LoadSynchronous();

        if (EffectData->SpawnType == EEffectSpawnType::Location)
        {
            NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), NS, Parameters.Location + LocOffset, RotOffset);
        }
        else if (EffectData->SpawnType == EEffectSpawnType::Attached && Mesh)
        {
            NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached( NS, Mesh, SocketName, LocOffset, RotOffset, EAttachLocation::SnapToTarget, true);
        }

        if (NiagaraComp && Parameters.RawMagnitude > 0.f && !ScaleParam.IsNone()) 
        {
            NiagaraComp->SetFloatParameter(ScaleParam, Parameters.RawMagnitude);
        }
    }
    
    if (!EffectData->Effect_SoundBase.IsNull())
    {
        USoundBase* SB = EffectData->Effect_SoundBase.LoadSynchronous();
        USoundAttenuation* SA = EffectData->Effect_Attenuation.IsValid() ? EffectData->Effect_Attenuation.LoadSynchronous() : nullptr;

        if (EffectData->SpawnType == EEffectSpawnType::Location)
        {
            
            AudioComp = UGameplayStatics::SpawnSoundAtLocation(this, SB, Parameters.Location + LocOffset, RotOffset, 1.f, 1.f, 0.f, SA);
        }
        else if (EffectData->SpawnType == EEffectSpawnType::Attached && Mesh)
        {
            AudioComp = UGameplayStatics::SpawnSoundAttached(SB, Mesh, SocketName, LocOffset, RotOffset, EAttachLocation::KeepRelativeOffset, false, 1.f, 1.f, 0.f, SA);
        }
    }

    return true; 
}

bool ASFGC_PlayCosmetic_Actor::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
    
    if (NiagaraComp)
    {
       NiagaraComp->Deactivate();
       NiagaraComp = nullptr;
    }
    
    if (AudioComp)
    {
       AudioComp->Stop(); 
       AudioComp = nullptr;
    }

    return true; 
}