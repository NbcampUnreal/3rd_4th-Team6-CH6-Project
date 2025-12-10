// Fill out your copyright notice in the Description page of Project Settings.


#include "SFGC_HitReaction.h"

#include "NiagaraFunctionLibrary.h"
#include "SFGameplayCueTags.h"
#include "Kismet/GameplayStatics.h"

void USFGC_HitReaction::HandleGameplayCue(
    AActor* Target,
    EGameplayCueEvent::Type EventType,
    const FGameplayCueParameters& Parameters)
{
    Super::HandleGameplayCue(Target, EventType, Parameters);

    if (!IsValid(Target) || EventType != EGameplayCueEvent::Executed)
        return;
    
    const FGameplayTag& TypeTag = Parameters.MatchedTagName;
    
    UNiagaraSystem* EffectToSpawn = nullptr;
    USoundBase* SoundToPlay = nullptr;

    
    bool bIsHeavy = Parameters.AggregatedSourceTags.HasTag(SFGameplayTags::GameplayCue_HitReaction_Heavy);
    bool bIsLight = Parameters.AggregatedSourceTags.HasTag(SFGameplayTags::GameplayCue_HitReaction_Light);
    

    // Intensity에 따라 이펙트 선택
    if (bIsHeavy)
    {
        EffectToSpawn = HeavyHitEffect;
        SoundToPlay = HeavyHitSound;
    }
    else if (bIsLight)
    {
        EffectToSpawn = LightHitEffect;
        SoundToPlay = LightHitSound;
    }
    else
    {
        // 기본값
        EffectToSpawn = LightHitEffect;
        SoundToPlay = LightHitSound;
    }
    
    // FX 재생
    if (EffectToSpawn)
    {
        FVector SpawnLoc = Parameters.Location;
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            Target->GetWorld(), 
            EffectToSpawn, 
            SpawnLoc, 
            FRotator::ZeroRotator,
            FVector(1.f),     
            true,             
            true              
        );
    }
    
    // Sound 재생
    if (SoundToPlay)
    {
        FVector Loc = Target->GetActorLocation();
        UGameplayStatics::PlaySoundAtLocation(Target->GetWorld(), SoundToPlay, Loc, VolumeMultiplier, PitchMultiplier);
    }
}