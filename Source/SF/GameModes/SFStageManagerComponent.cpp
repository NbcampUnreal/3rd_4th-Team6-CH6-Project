#include "SFStageManagerComponent.h"

#include "Net/UnrealNetwork.h"
#include "System/SFStageSubsystem.h"

USFStageManagerComponent::USFStageManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

void USFStageManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ThisClass, CurrentStageInfo);
    DOREPLIFETIME(ThisClass, bStageCleared);
}

void USFStageManagerComponent::BeginPlay()
{
    Super::BeginPlay();

    // 서버에서만 Subsystem에서 현재 스테이지 정보 가져오기
    if (GetOwner()->HasAuthority())
    {
        if (UGameInstance* GI = GetWorld()->GetGameInstance())
        {
            if (USFStageSubsystem* StageSubsystem = GI->GetSubsystem<USFStageSubsystem>())
            {
                CurrentStageInfo = StageSubsystem->GetCurrentStageInfo();
            }
        }
    }
}

void USFStageManagerComponent::NotifyStageClear()
{
    if (!GetOwner()->HasAuthority())
    {
        return;
    }

    if (bStageCleared)
    {
        return;
    }

    bStageCleared = true;

    OnStageCleared.Broadcast(CurrentStageInfo);
}

void USFStageManagerComponent::OnRep_bStageCleared()
{
    if (bStageCleared)
    {
        OnStageCleared.Broadcast(CurrentStageInfo);
    }
}




