#include "SFGameState.h"
#include "SFPortalManagerComponent.h"

ASFGameState::ASFGameState()
{
	// Portal Manager 생성
	PortalManager = CreateDefaultSubobject<USFPortalManagerComponent>(TEXT("PortalManager"));
}

