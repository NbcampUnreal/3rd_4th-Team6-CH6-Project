#include "SFCharacterMovementReplication.h"
#include "SFHeroMovementComponent.h"

void FSFCharacterNetworkMoveData::ClientFillNetworkMoveData(const FSavedMove_Character& ClientMove, ENetworkMoveType MoveType)
{
	Super::ClientFillNetworkMoveData(ClientMove, MoveType);

	const FSavedMove_SFCharacter& SFMove = static_cast<const FSavedMove_SFCharacter&>(ClientMove);

	NetworkWarpTargetLocation = SFMove.SavedWarpTargetLocation;
	NetworkWarpTargetRotation = SFMove.SavedWarpTargetRotation;
	bNetworkHasWarpTarget = SFMove.bSavedHasWarpTarget;
}

bool FSFCharacterNetworkMoveData::Serialize(UCharacterMovementComponent& CharacterMovement, FArchive& Ar, UPackageMap* PackageMap, ENetworkMoveType MoveType)
{
	if (!Super::Serialize(CharacterMovement, Ar, PackageMap, MoveType))
	{
		return false;
	}

	// Warp 타겟 데이터 직렬화
	Ar << bNetworkHasWarpTarget;

	if (bNetworkHasWarpTarget)
	{
		// 대역폭 최적화: 압축 직렬화 사용
		bool bSuccess = true;
		NetworkWarpTargetLocation.NetSerialize(Ar, PackageMap, bSuccess);
		NetworkWarpTargetRotation.NetSerialize(Ar, PackageMap, bSuccess);
	}

	return !Ar.IsError();
}

FSFCharacterNetworkMoveDataContainer::FSFCharacterNetworkMoveDataContainer()
{
	NewMoveData = &SFDefaultMoveData[0];
	PendingMoveData = &SFDefaultMoveData[1];
	OldMoveData = &SFDefaultMoveData[2];
}