#pragma once

#include "CoreMinimal.h"
#include "SFMenuPlayerController.h"
#include "SFLobbyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class SF_API ASFLobbyPlayerController : public ASFMenuPlayerController
{
	GENERATED_BODY()

public:
	ASFLobbyPlayerController();
	virtual void SetupInputComponent() override;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestStartMatch();

	UFUNCTION(BlueprintCallable)
	void ToggleReady();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USFInGameMenuComponent> InGameMenuComponent;
};
