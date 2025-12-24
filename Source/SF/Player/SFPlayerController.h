#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "System/Data/SFStageInfo.h"
#include "SFPlayerController.generated.h"

struct FSFStageInfo;
class USFSkillSelectionScreen;
class USFLoadingCheckComponent;
class ASFPlayerState;
class USFAbilitySystemComponent;
class UUserWidget;
class UInputAction;
class UInputMappingContext;

/**
 * 
 */
UCLASS()
class SF_API ASFPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ASFPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~AController interface
	virtual void BeginPlay() override;
	virtual void OnUnPossess() override;
	virtual void OnRep_PlayerState() override;
	virtual void SetupInputComponent() override; 
	//~End of AController interface

	UFUNCTION(BlueprintCallable, Category = "SF|PlayerController")
	ASFPlayerState* GetSFPlayerState() const;
	
	UFUNCTION(BlueprintCallable, Category = "SF|PlayerController")
	USFAbilitySystemComponent* GetSFAbilitySystemComponent() const;

protected:
	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;

protected:
	// ----------------[추가] 인게임 메뉴 관련 변수 및 함수----------------------

	// 에디터 상에서 지정할 IA_InGameMenu 변수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Input")
	TObjectPtr<UInputAction> InGameMenuAction;

	// 에디터 상에서 지정할 IMC 변수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|InGame")
	TSubclassOf<UUserWidget> InGameMenuClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> InGameMenuInstance;

	// 팀원 표시 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|InGame")
	TSubclassOf<UUserWidget> TeammateIndicatorWidgetClass;

	// 생성된 팀원 표시 위젯 관리 맵
	UPROPERTY()
	TMap<AActor*, class USFIndicatorWidgetBase*> TeammateWidgetMap;

	// 팀원 표시 검색 타이머 핸들
	FTimerHandle TeammateSearchTimerHandle;
	

protected:
	// 인게임 메뉴 생성 함수
	void ToggleInGameMenu();
	// 팀원 위젯 생성 함수
	void CreateTeammateIndicators();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|SkillSelection")
	TSubclassOf<USFSkillSelectionScreen> SkillSelectionScreenClass;

	UPROPERTY()
	TObjectPtr<USFSkillSelectionScreen> SkillSelectionScreenInstance;

	UFUNCTION()
	void HandleStageCleared(const FSFStageInfo& ClearedStageInfo);

	UFUNCTION()
	void OnSkillSelectionComplete();

	void ShowSkillSelectionScreen(int32 StageIndex);
	void HideSkillSelectionScreen();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SF|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USFLoadingCheckComponent> LoadingCheckComponent;

	// 스테이지 클리어 대기 (PawnData 미로드 시)
	bool bPendingStageCleared = false;
	FSFStageInfo PendingStageInfo;
	FDelegateHandle PawnDataLoadedHandle;
};
