#include "SFBTComposite_Random.h"

USFBTComposite_Random::USFBTComposite_Random(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = "Random Selector"; // 에디터에 표시될 이름
}

int32 USFBTComposite_Random::GetNextChildHandler(struct FBehaviorTreeSearchData& SearchData, int32 PrevChild, EBTNodeResult::Type LastResult) const
{
	// 1. 이미 자식을 실행하고 돌아온 경우 (성공이든 실패든) -> 부모에게 결과 보고 (종료)
	if (PrevChild != BTSpecialChild::NotInitialized)
	{
		return BTSpecialChild::ReturnToParent;
	}

	// 2. 처음 진입한 경우 -> 자식이 없으면 종료
	if (Children.Num() == 0)
	{
		return BTSpecialChild::ReturnToParent;
	}

	// 3. 자식들 중 하나를 '완전 랜덤'으로 뽑아서 실행
	// (0번부터 자식 개수 - 1 사이의 숫자 뽑기)
	int32 RandomIndex = FMath::RandRange(0, Children.Num() - 1);
	
	return RandomIndex;
}