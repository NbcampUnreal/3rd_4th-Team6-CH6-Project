#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputActionValue.h"
#include "SFInGameMenuComponent.generated.h"

class UInputAction;
class UInputMappingContext;
class UEnhancedInputComponent;

/**
 * 인게임 메뉴(ESC) 기능을 담당하는 컴포넌트
 *  PlayerController부착 -> 인게임 메뉴 기능을 제공
 */

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SF_API USFInGameMenuComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	USFInGameMenuComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// PC의 SetupInputComponent에서 호출
	void SetupInputBindings(UEnhancedInputComponent* PlayerInputComponent);

	// 메뉴 토글
	UFUNCTION(BlueprintCallable, Category = "UI|Menu")
	void ToggleInGameMenu();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Menu")
	TObjectPtr<UInputAction> InGameMenuAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Menu")
	TObjectPtr<UInputMappingContext> InGameMenuMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Menu")
	TSubclassOf<class USFInGameMenuWidget> InGameMenuClass;

private:
	UPROPERTY()
	TObjectPtr<class USFInGameMenuWidget> InGameMenuInstance;
};
