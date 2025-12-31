// Fill out your copyright notice in the Description page of Project Settings.


#include "SFDragon.h"

#include "DragonMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Physics/SFCollisionChannels.h"


// Sets default values
ASFDragon::ASFDragon()
{
	// 1. 캡슐 설정 수정 (이동의 핵심)
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
    
	// 바닥 감지를 위해 반드시 필요한 채널들
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block); // 동적 바닥 대비
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);   // 바닥 감지 트레이스용
    
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); 

	// 2. 메쉬 설정 (피격 및 물리 충돌용)
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionObjectType(ECC_Pawn);
	GetMesh()->SetCollisionResponseToAllChannels(ECR_Ignore);
    
	// 메쉬도 바닥과 충돌은 하되, 이동은 캡슐이 주도함
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    
	// 무기 채널 설정
	GetMesh()->SetCollisionResponseToChannel(SF_ObjectChannel_Weapon, ECR_Block); 
    
	GetMesh()->SetUseCCD(true);
	GetMesh()->SetGenerateOverlapEvents(true); 
    
	DragonMovementComponent = CreateDefaultSubobject<USFDragonMovementComponent>(TEXT("DragonMovementComponent"));
	DragonMovementComponent->SetIsReplicated(true);
}

void ASFDragon::InitializeMovementComponent()
{
	Super::InitializeMovementComponent();

	if (IsValid(DragonMovementComponent))
	{
		DragonMovementComponent->InitializeDragonMovementComponent();
	}
}

void ASFDragon::InitializeComponents()
{
	Super::InitializeComponents();

}







