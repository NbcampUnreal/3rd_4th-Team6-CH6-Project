#include "SFPlayerSlot.h"
#include "Components/ArrowComponent.h"
#include "Character/Hero/SFHeroDefinition.h"
#include "Actors/SFHeroDisplay.h"
#include "SFLogChannels.h"
#include "Player/SFPlayerState.h"

ASFPlayerSlot::ASFPlayerSlot()
{
	PrimaryActorTick.bCanEverTick = false;

	// Root
	USceneComponent* RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(RootComp);

	// Arrow
	Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	Arrow->SetupAttachment(RootComp);

	// 초기화
	CachedPC = nullptr;
	HeroDisplay = nullptr;
}

void ASFPlayerSlot::BeginPlay()
{
	Super::BeginPlay();
	
	// HeroDisplay 미리 스폰
	SpawnHeroDisplay();
}

void ASFPlayerSlot::SpawnHeroDisplay()
{
	if (HeroDisplay)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Arrow 위치에 HeroDisplay 스폰
	FTransform SpawnTransform = Arrow->GetComponentTransform();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Owner = this;

	if (HeroDisplayClass)
	{
		HeroDisplay = World->SpawnActor<ASFHeroDisplay>(
			HeroDisplayClass, 
			SpawnTransform, 
			SpawnParams
		);
	}
	else
	{
		UE_LOG(LogSF, Error, TEXT("[PlayerSlot] HeroDisplayClass is null"));
	}
	
	if (!HeroDisplay)
	{
		UE_LOG(LogSF, Error, TEXT("[PlayerSlot] Failed to spawn HeroDisplay"));
		return;
	}

	// 초기에는 숨김
	HeroDisplay->SetActorHiddenInGame(true);
}

void ASFPlayerSlot::AddPlayer(APlayerController* InPC, const USFHeroDefinition* HeroDefinition)
{
	// PC 저장
	CachedPC = InPC;
	
	if (!CachedPC.IsValid())
	{
		return;
	}

	if (ASFPlayerState* SFPlayerState = CachedPC->GetPlayerState<ASFPlayerState>())
	{
		const USFHeroDefinition* HeroDef  = SFPlayerState->GetPlayerSelection().GetHeroDefinition();
		UpdateHeroDisplay(HeroDef);
	}
}

void ASFPlayerSlot::RemovePlayer(APlayerController* PC)
{
	CachedPC = nullptr;
	
	if (!HeroDisplay)
	{
		return;
	}

	// HeroDisplay 숨김
	HeroDisplay->SetActorHiddenInGame(true);

	UE_LOG(LogSF, Log, TEXT("[PlayerSlot] RemovePlayer"));
}

void ASFPlayerSlot::SwitchHeroDefinition(const USFHeroDefinition* HeroDefinition)
{
	UpdateHeroDisplay(HeroDefinition);
}

void ASFPlayerSlot::UpdateHeroDisplay(const USFHeroDefinition* HeroDefinition) const
{
	if (!HeroDisplay || !HeroDefinition)
	{
		return;
	}

	HeroDisplay->ConfigureWithHeroDefination(HeroDefinition);
	HeroDisplay->SetActorTransform(Arrow->GetComponentTransform());
	HeroDisplay->SetActorHiddenInGame(false);
}


