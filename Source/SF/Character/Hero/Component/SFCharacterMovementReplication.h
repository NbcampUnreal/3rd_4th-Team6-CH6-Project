#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"

// 커스텀 네트워크 데이터 컨테이너(직렬화 담당)
class FSFCharacterNetworkMoveData : public FCharacterNetworkMoveData
{
public:
	typedef FCharacterNetworkMoveData Super;

	FSFCharacterNetworkMoveData()
		: NetworkWarpTargetLocation(FVector::ZeroVector)
		, NetworkWarpTargetRotation(FRotator::ZeroRotator)
		, bNetworkHasWarpTarget(false)
	{
	}

	/**
	 * UCharacterMovementComponent에서 반환된 FSavedMove_Character 구조체를 사용하여 관련 이동 데이터를 채워 넣음.
	 * 참고로, 저장된 이동 데이터를 추가한 경우 FSavedMove_Character 인스턴스는 사용자가 만든 파생 구조체의 사용자 정의 구조체일 수 있음.
	 * @see UCharacterMovementComponent::AllocateNewMove()
	 */
	virtual void ClientFillNetworkMoveData(const FSavedMove_Character& ClientMove, ENetworkMoveType MoveType) override;

	/**
	 * 이 구조체의 데이터를 지정된 FArchive로 직렬화하거나 지정된 FArchive에서 직렬화. 이렇게 하면 데이터가 가변 크기의 데이터 스트림으로 압축되거나 압축 해제되어 클라이언트에서 서버로 네트워크를 통해 전송
	 * @see UCharacterMovementComponent::CallServerMovePacked
	 */
	virtual bool Serialize(UCharacterMovementComponent& CharacterMovement, FArchive& Ar, UPackageMap* PackageMap, ENetworkMoveType MoveType) override;

	FVector NetworkWarpTargetLocation;
	FRotator NetworkWarpTargetRotation;
	bool bNetworkHasWarpTarget;
};

class SF_API FSFCharacterNetworkMoveDataContainer : public FCharacterNetworkMoveDataContainer
{
public:
	FSFCharacterNetworkMoveDataContainer();

	FSFCharacterNetworkMoveData SFDefaultMoveData[3];
};