#include "SFTeamInfoStatics.h"

#include "AbilitySystemGlobals.h"
#include "SFTeamTypes.h"
#include "Player/SFPlayerState.h"

FGenericTeamId USFTeamInfoStatics::GetTeamIdFromActor(const AActor* Actor)
{
	if (const IGenericTeamAgentInterface* TeamAgent = FindTeamAgentFromObject(Actor))
	{
		return TeamAgent->GetGenericTeamId();
	}
	return FGenericTeamId::NoTeam;
}

bool USFTeamInfoStatics::AreHostile(const AActor* A, const AActor* B)
{
	if (!A || !B)
	{
		return false;
	}

	FGenericTeamId TeamA = GetTeamIdFromActor(A);
	FGenericTeamId TeamB = GetTeamIdFromActor(B);

	return FGenericTeamId::GetAttitude(TeamA, TeamB) == ETeamAttitude::Hostile;
}

bool USFTeamInfoStatics::AreAlly(const AActor* A, const AActor* B)
{
	if (!A || !B)
	{
		return false;
	}

	FGenericTeamId TeamA = GetTeamIdFromActor(A);
	FGenericTeamId TeamB = GetTeamIdFromActor(B);

	return FGenericTeamId::GetAttitude(TeamA, TeamB) == ETeamAttitude::Friendly;
}

bool USFTeamInfoStatics::IsPlayerTeam(const AActor* Actor)
{
	return GetTeamIdFromActor(Actor) == FGenericTeamId(SFTeamID::Player);
}

bool USFTeamInfoStatics::IsEnemyTeam(const AActor* Actor)
{
	return GetTeamIdFromActor(Actor) == FGenericTeamId(SFTeamID::Enemy);
}

const IGenericTeamAgentInterface* USFTeamInfoStatics::FindTeamAgentFromObject(const UObject* TestObject)
{
	if (!TestObject)
	{
		return nullptr;
	}

	// TeamAgent인 경우
	if (const IGenericTeamAgentInterface* DirectTeamAgent = Cast<IGenericTeamAgentInterface>(TestObject))
	{
		return DirectTeamAgent;
	}

	// Actor인 경우 추가 탐색
	if (const AActor* TestActor = Cast<const AActor>(TestObject))
	{
		// Pawn이면 Controller 체크 (Enemy AI용)
		if (const APawn* Pawn = Cast<const APawn>(TestActor))
		{
			if (const IGenericTeamAgentInterface* ControllerTeamAgent = 
				Cast<IGenericTeamAgentInterface>(Pawn->GetController()))
			{
				return ControllerTeamAgent;
			}
		}

		// Instigator 체크 (투사체 등)
		if (const IGenericTeamAgentInterface* InstigatorTeamAgent = 
			Cast<IGenericTeamAgentInterface>(TestActor->GetInstigator()))
		{
			return InstigatorTeamAgent;
		}

		// Owner 체크
		if (const IGenericTeamAgentInterface* OwnerTeamAgent = 
			Cast<IGenericTeamAgentInterface>(TestActor->GetOwner()))
		{
			return OwnerTeamAgent;
		}

		// PlayerState 체크
		if (const IGenericTeamAgentInterface* PSTeamAgent = 
			Cast<IGenericTeamAgentInterface>(FindPlayerStateFromActor(TestActor)))
		{
			return PSTeamAgent;
		}
	}

	return nullptr;
}

const ASFPlayerState* USFTeamInfoStatics::FindPlayerStateFromActor(const AActor* PossibleTeamActor)
{
	if (PossibleTeamActor)
	{
		if (const APawn* Pawn = Cast<const APawn>(PossibleTeamActor))
		{
			if (ASFPlayerState* SFPS = Pawn->GetPlayerState<ASFPlayerState>())
			{
				return SFPS;
			}
		}
		else if (const AController* Controller = Cast<const AController>(PossibleTeamActor))
		{
			if (ASFPlayerState* SFPS = Cast<ASFPlayerState>(Controller->PlayerState))
			{
				return SFPS;
			}
		}
		else if (const ASFPlayerState* SFPS = Cast<const ASFPlayerState>(PossibleTeamActor))
		{
			return SFPS; 
		}
	}

	return nullptr;
}

bool USFTeamInfoStatics::CanCauseDamage(const AActor* Instigator, const AActor* Target, bool bAllowSelfDamage)
{
	if (Instigator == nullptr || Target == nullptr)
	{
		return false;
	}
	
	if (bAllowSelfDamage)
	{
		if ((Instigator == Target) || (FindPlayerStateFromActor(Instigator) == FindPlayerStateFromActor(Target)))
		{
			return true;
		}
	}
	
	const IGenericTeamAgentInterface* InstigatorTeamAgent = FindTeamAgentFromObject(Instigator);
	const IGenericTeamAgentInterface* TargetTeamAgent = FindTeamAgentFromObject(Target);
	if (InstigatorTeamAgent == nullptr || TargetTeamAgent == nullptr)
	{
		return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target) != nullptr;
	}
	
	ETeamAttitude::Type TeamAttitudeType = FGenericTeamId::GetAttitude(InstigatorTeamAgent->GetGenericTeamId(), TargetTeamAgent->GetGenericTeamId());
	return (TeamAttitudeType == ETeamAttitude::Hostile);
}
