// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Hero/SFHero.h"

#include "Player/SFPlayerState.h"
#include "Team/SFTeamTypes.h"

FGenericTeamId ASFHero::GetGenericTeamId() const
{
	if (const ASFPlayerState* PS = GetPlayerState<ASFPlayerState>())
	{
		return PS->GetGenericTeamId();
	}
	return FGenericTeamId(SFTeamID::Player);
}
