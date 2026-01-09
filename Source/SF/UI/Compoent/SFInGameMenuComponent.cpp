#include "UI/Compoent/SFInGameMenuComponent.h"

#include "UI/InGame/SFInGameMenuWidget.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"


USFInGameMenuComponent::USFInGameMenuComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void USFInGameMenuComponent::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (InGameMenuMappingContext)
			{
				Subsystem->AddMappingContext(InGameMenuMappingContext, 100);
			}
		}
	}
}

void USFInGameMenuComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 레벨 전환되거나 PC 파괴될 때 위젯 정리
	if (InGameMenuInstance)
	{
		if (InGameMenuInstance->IsInViewport())
		{
			InGameMenuInstance->RemoveFromParent();
		}
		InGameMenuInstance = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}

void USFInGameMenuComponent::SetupInputBindings(UEnhancedInputComponent* PlayerInputComponent)
{
	if (!PlayerInputComponent || !InGameMenuAction) return;

	// 인풋 액션 바인딩
	PlayerInputComponent->BindAction(InGameMenuAction, ETriggerEvent::Started, this, &USFInGameMenuComponent::ToggleInGameMenu);
}

void USFInGameMenuComponent::ToggleInGameMenu()
{
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC) return;

	UE_LOG(LogTemp, Log, TEXT("[SFSystemMenuComponent] Toggle Menu Request"));

	// 1. 이미 켜져 있으면 끄기
	if (InGameMenuInstance && InGameMenuInstance->IsInViewport())
	{
		InGameMenuInstance->RemoveFromParent();
		InGameMenuInstance = nullptr;

		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
		return;
	}
	
	// 2. 꺼져 있으면 켜기
	if (InGameMenuClass)
	{
		InGameMenuInstance = CreateWidget<USFInGameMenuWidget>(PC, InGameMenuClass);
		if (InGameMenuInstance)
		{
			InGameMenuInstance->AddToViewport(100);

			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(InGameMenuInstance->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = true;
		}
	}
}


