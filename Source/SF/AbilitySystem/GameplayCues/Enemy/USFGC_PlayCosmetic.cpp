// Fill out your copyright notice in the Description page of Project Settings.


#include "USFGC_PlayCosmetic.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Character/SFPawnExtensionComponent.h"
#include "Character/Enemy/SFEnemy.h"
#include "Character/Enemy/SFEnemyData.h"
#include "Character/Enemy/SF_EffectConfig.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

// USFGC_PlayCosmetic.cpp

#include "USFGC_PlayCosmetic.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Character/SFPawnExtensionComponent.h"
#include "Character/Enemy/SFEnemyData.h"
#include "Character/Enemy/SF_EffectConfig.h"
#include "AbilitySystem/GameplayCues/Data/SFGameplayCueCosmeticData.h"

void UUSFGC_PlayCosmetic::HandleGameplayCue(AActor* Target, EGameplayCueEvent::Type EventType, const FGameplayCueParameters& Parameters)
{
    Super::HandleGameplayCue(Target, EventType, Parameters);
    if (EventType != EGameplayCueEvent::Executed) return;
    
    ASFCharacterBase* Character = Cast<ASFCharacterBase>(Target);
    if (!Character) return;

    USFPawnExtensionComponent* PawnExtComp = USFPawnExtensionComponent::FindPawnExtensionComponent(Character);
    if (!PawnExtComp) return;

    const USFEnemyData* EnemyData = Cast<USFEnemyData>(PawnExtComp->GetPawnData<USFPawnData>());
    if (!EnemyData || !EnemyData->EffectConfig) return;
    
    const FEffectData* EffectData = EnemyData->EffectConfig->EffectDataMap.Find(Parameters.MatchedTagName);
    if (!EffectData) return;
    
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
    
    
    USkeletalMeshComponent* Mesh = Character->GetMesh();
    
    if (!EffectData->Effect_NiagaraSystem.IsNull())
    {
        UNiagaraSystem* NS = EffectData->Effect_NiagaraSystem.LoadSynchronous();
        UNiagaraComponent* NC = nullptr;

        if (EffectData->SpawnType == EEffectSpawnType::Location)
        {
        
            FVector FinalLocation = Parameters.Location + LocOffset;
            NC = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), NS, FinalLocation, RotOffset);
        }
        else if (EffectData->SpawnType == EEffectSpawnType::Attached && Mesh)
        {
        
            NC = UNiagaraFunctionLibrary::SpawnSystemAttached(
                NS, Mesh, SocketName, LocOffset, RotOffset, EAttachLocation::SnapToTarget, true
            );
        }
        
        if (NC && Parameters.RawMagnitude > 0.f && !ScaleParam.IsNone())
        {
            NC->SetFloatParameter(ScaleParam, Parameters.RawMagnitude);
        }
    }

    
    if (!EffectData->Effect_SoundBase.IsNull())
    {
        USoundBase* SB = EffectData->Effect_SoundBase.LoadSynchronous();
        USoundAttenuation* SA = EffectData->Effect_Attenuation.IsValid() ? EffectData->Effect_Attenuation.LoadSynchronous() : nullptr;

        if (EffectData->SpawnType == EEffectSpawnType::Location)
        {
             UGameplayStatics::PlaySoundAtLocation(
                 this, SB, Parameters.Location + LocOffset, 1.f, 1.f, 0.f, SA
             );
        }
        else if (EffectData->SpawnType == EEffectSpawnType::Attached && Mesh)
        {
             UGameplayStatics::SpawnSoundAttached(
                 SB, Mesh, SocketName, LocOffset, RotOffset, EAttachLocation::KeepRelativeOffset, 
                 true, 
                 1.f, 1.f, 0.f, SA
             );
        }
    }
}