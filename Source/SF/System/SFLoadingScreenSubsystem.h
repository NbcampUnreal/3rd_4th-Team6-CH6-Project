// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SFLoadingScreenSubsystem.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPreLoadingScreenWidgetChangedDelegate, TSubclassOf<UUserWidget>, NewWidgetClass);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLoadingScreenWidgetChangedDelegate, TSubclassOf<UUserWidget>, NewWidgetClass);

/**
 * 
 */
UCLASS(config = Game)
class SF_API USFLoadingScreenSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Hard Travel 로딩 스크린 위젯 설정
	UFUNCTION(BlueprintCallable, Category = "SF|Loading")
	void SetPreLoadingScreenContentWidget(TSubclassOf<UUserWidget> NewWidgetClass);

	// 현재 설정된 Hard Travel 로딩 스크린 위젯 클래스 리턴
	UFUNCTION(BlueprintPure, Category = "SF|Loading")
	TSubclassOf<UUserWidget> GetPreLoadingScreenContentWidget() const;
	
	// Seamless Travel 로딩 스크린 위젯 설정
	UFUNCTION(BlueprintCallable, Category = "SF|Loading")
	void SetLoadingScreenContentWidget(TSubclassOf<UUserWidget> NewWidgetClass);

	// 현재 설정된 Seamless Travel 로딩 스크린 위젯 클래스 리턴
	UFUNCTION(BlueprintPure, Category = "SF|Loading")
	TSubclassOf<UUserWidget> GetLoadingScreenContentWidget() const;
	
	// 수동으로 로딩 스크린 시작 
	UFUNCTION(BlueprintCallable, Category = "SF|Loading")
	void StartLoadingScreen();
	
	// 수동으로 로딩 스크린 종료
	UFUNCTION(BlueprintCallable, Category = "SF|Loading")
	void StopLoadingScreen();

private:
	// 하드 트래블 감지용 (OpenLevel)
	void OnPreLoadMap(const FString& MapName);

	// 맵 로딩 완료 감지
	void OnPostLoadMapWithWorld(UWorld* LoadedWorld);

	bool IsInSeamlessTravel();
	
	// 트랜지션 맵인지 확인하는 헬퍼 함수
	bool IsTransitionMap(const FString& MapName);

	void RemoveWidgetFromViewport();

private:
	bool bCurrentLoadingScreenStarted;
	
	UPROPERTY(Config, EditDefaultsOnly, Category = "SF|Loading")
	FSoftClassPath PreLoadingScreenWidget;

	TSharedPtr<SWidget> PreLoadingSWidgetPtr;

	UPROPERTY(BlueprintAssignable, meta=(AllowPrivateAccess))
	FLoadingScreenWidgetChangedDelegate OnPreLoadingScreenWidgetChanged;
	
	UPROPERTY(BlueprintAssignable, meta=(AllowPrivateAccess))
	FLoadingScreenWidgetChangedDelegate OnLoadingScreenWidgetChanged;

	UPROPERTY()
	TSubclassOf<UUserWidget> PreLoadingScreenWidgetClass;
	
	UPROPERTY()
	TSubclassOf<UUserWidget> LoadingScreenWidgetClass;
};
