// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SFSettingsSaveGame.generated.h"

/**
 * 유저 설정을 저장할 데이터 클래스
 */
UCLASS()
class SF_API USFSettingsSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// [소리 설정]
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Settings")
	float MasterVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Settings")
	float BGMVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Settings")
	float SFXVolume = 1.0f;

	// [게임플레이 설정]
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Settings")
	float MouseSensitivity = 0.5f;

	// [화면 설정]
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Settings")
	float Brightness = 0.5f;

	// (0: 전체화면(Borderless), 1: 창모드)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Settings")
	int32 WindowModeIndex = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Settings")
	int32 WindowMode;

	// (0: 1280x720, 1: 1920x1080, 2: 2560x1440, 3: 3840x2160)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Settings")
	int32 ResolutionIndex = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Settings")
	FIntPoint Resolution;
	
};
