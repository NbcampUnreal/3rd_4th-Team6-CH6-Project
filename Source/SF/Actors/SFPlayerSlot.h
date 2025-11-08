// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SFPlayerSlot.generated.h"

class USFHeroDefinition;
class ASFHeroDisplay;
class APlayerController;
class UArrowComponent;

UCLASS()
class SF_API ASFPlayerSlot : public AActor
{
	GENERATED_BODY()
	
public:
	ASFPlayerSlot();

	/**
	 * AddPlayer - Hero 표시
	 * @param PC - PlayerController
	 * @param HeroDefinition - 표시할 Hero
	 */
	UFUNCTION(BlueprintCallable, Category = "PlayerSlot")
	void AddPlayer(APlayerController* PC, const USFHeroDefinition* HeroDefinition);

	/**
	 * RemovePlayer - Hero 숨김
	 */
	UFUNCTION(BlueprintCallable, Category = "PlayerSlot")
	void RemovePlayer(APlayerController* PC);

	void SwitchHeroDefinition(const USFHeroDefinition* HeroDefinition);
	void UpdateHeroDisplay(const USFHeroDefinition* HeroDefinition) const;

	FORCEINLINE uint8 GetSlotID() const { return SlotID; }
	FORCEINLINE APlayerController* GetCurrentPC() const { return CachedPC.Get(); }
	FORCEINLINE bool HasValidPC() const { return CachedPC.IsValid(); }
	FORCEINLINE ASFHeroDisplay* GetHeroDisplay() const { return HeroDisplay; }

protected:
	virtual void BeginPlay() override;

private:
	/** HeroDisplay 스폰 (BeginPlay에서 한 번만) */
	void SpawnHeroDisplay();

private:
	/** Arrow Component */
	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	TObjectPtr<UArrowComponent> Arrow;

	UPROPERTY(EditDefaultsOnly, Category = "Display")
	TSubclassOf<ASFHeroDisplay> HeroDisplayClass;

	/** HeroDisplay 액터 (재사용) */
	UPROPERTY()
	TObjectPtr<ASFHeroDisplay> HeroDisplay;

	UPROPERTY()
	FPrimaryAssetId HeroDefinitionAssetID;

	/** PC 변수 */
	UPROPERTY()
	TWeakObjectPtr<APlayerController> CachedPC;

	/** 레벨에서 설정할 슬롯 번호 (0부터 시작) */
	UPROPERTY(EditInstanceOnly, Category = "PlayerSlot", meta = (ClampMin = "0", ClampMax = "9"))
	uint8 SlotID;
};
