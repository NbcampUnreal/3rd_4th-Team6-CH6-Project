// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Hero/SFHero.h"

#include "Component/SFHeroMovementComponent.h"
#include "Player/SFPlayerController.h"
#include "Player/SFPlayerState.h"
#include "Team/SFTeamTypes.h"

ASFHero::ASFHero(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<USFHeroMovementComponent>(CharacterMovementComponentName))
{
}

ASFPlayerController* ASFHero::GetSFPlayerController() const
{
	return Cast<ASFPlayerController>(Controller);
}

FGenericTeamId ASFHero::GetGenericTeamId() const
{
	if (const ASFPlayerState* PS = GetPlayerState<ASFPlayerState>())
	{
		return PS->GetGenericTeamId();
	}
	return FGenericTeamId(SFTeamID::Player);
}
