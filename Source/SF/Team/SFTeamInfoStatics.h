// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SFTeamInfoStatics.generated.h"

class ASFPlayerState;
/**
 * 
 */
UCLASS()
class SF_API USFTeamInfoStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "SF|Team")
	static FGenericTeamId GetTeamIdFromActor(const AActor* Actor);

	// 두 Actor가 적대 관계인지
	UFUNCTION(BlueprintPure, Category = "SF|Team")
	static bool AreHostile(const AActor* A, const AActor* B);

	// 두 Actor가 같은 팀인지
	UFUNCTION(BlueprintPure, Category = "SF|Team")
	static bool AreAlly(const AActor* A, const AActor* B);

	// Actor가 플레이어 팀인지
	UFUNCTION(BlueprintPure, Category = "SF|Team")
	static bool IsPlayerTeam(const AActor* Actor);

	// Actor가 적 팀인지
	UFUNCTION(BlueprintPure, Category = "SF|Team")
	static bool IsEnemyTeam(const AActor* Actor);

	static const IGenericTeamAgentInterface* FindTeamAgentFromObject(const UObject* TestObject);

	UFUNCTION(BlueprintPure, Category = "SF|Team")
	static const ASFPlayerState* FindPlayerStateFromActor(const AActor* PossibleTeamActor);

	UFUNCTION(BlueprintPure, Category = "SF|Team")
	static bool CanCauseDamage(const AActor* Instigator, const AActor* Target, bool bAllowSelfDamage = false);
};
