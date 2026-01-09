#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "SFCreditStruct.generated.h"

/**
 * 크레딧 화면에 표시할 데이터 구조체
 * (CSV 파일 연동)
 */

USTRUCT(BlueprintType)
struct FSFCreditRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	// [공통] 섹션 구분 (예: "Core Team", "External Assets")
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Credits")
	FString SectionTitle;

	// [공통] 역할/분류 (예: "UI Programming", "BGM", "Font")
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Credits")
	FString RoleOrCategory;

	// [공통] 이름/저작자 (예: "이정국", "Kevin MacLeod")
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Credits")
	FString NameOrAuthor;

	// 자산 제목 (예: "Monkeys Spinning Monkeys")
	// 팀원일 경우 입력 X
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Credits")
	FString AssetName;

	// 자산 설명
	// 팀원일 경우 입력 X
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Credits")
	FString AssetDescription;

	// 출처/라이선스 링크 (예: "http://creativecommons.org...")
	// 팀원일 경우 입력 X
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Credits")
	FString SourceURL;
};
