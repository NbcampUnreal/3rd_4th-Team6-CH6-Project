#pragma once

#include "CoreMinimal.h"
#include "UI/InGame/SFDamageWidget.h"
#include "UIDataStructs.generated.h"

USTRUCT(BlueprintType)
struct FSFDamageMessageInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "UI|InGame")
	AActor* TargetActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "UI|InGame")
	AActor* Instigator = nullptr;
	
	UPROPERTY(BlueprintReadOnly, Category = "UI|InGame")
	float DamageAmount = 0.f; // 데미지 양
	
	UPROPERTY(BlueprintReadOnly, Category = "UI|InGame")
	TWeakObjectPtr<USFDamageWidget> ActiveWidget = nullptr; // 현재 떠 있는 위젯 (약참조)
};