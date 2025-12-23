#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTCompositeNode.h"
#include "SFBTComposite_Random.generated.h"

/**
 * 자식 노드들 중 하나를 무작위로 선택해서 실행하는 컴포짓 노드
 */
UCLASS()
class SF_API USFBTComposite_Random : public UBTCompositeNode
{
	GENERATED_BODY()

public:
	USFBTComposite_Random(const FObjectInitializer& ObjectInitializer);

	// 비헤이비어 트리 로직: 다음에 실행할 자식 찾기
	virtual int32 GetNextChildHandler(struct FBehaviorTreeSearchData& SearchData, int32 PrevChild, EBTNodeResult::Type LastResult) const override;
};