#include "SFSpectatorPawn.h"

#include "Camera/SFCameraComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Player/SFPlayerState.h"

ASFSpectatorPawn::ASFSpectatorPawn(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    PrimaryActorTick.TickGroup = TG_PostPhysics;

    SetReplicateMovement(false);

    if (GetMovementComponent())
    {
        GetMovementComponent()->Deactivate();
    }

    CameraComponent = CreateDefaultSubobject<USFCameraComponent>(TEXT("CameraComponent"));
    CameraComponent->SetupAttachment(RootComponent);
}

void ASFSpectatorPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    // 부모 호출 안 함 - 기본 WASD/마우스 입력 바인딩 방지
    //Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ASFSpectatorPawn::BeginPlay()
{
    Super::BeginPlay();

    // 카메라 모드 델리게이트 바인딩
    if (CameraComponent)
    {
        CameraComponent->DetermineCameraModeDelegate.BindDynamic(this, &ThisClass::DetermineCameraMode);
    }
}

void ASFSpectatorPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    AActor* Target = FollowTarget.Get();
    if (!Target)
    {
        return;
    }

    // 위치 동기화(타겟 위치로 부드럽게 이동)
    FVector TargetLocation = Target->GetActorLocation();
    FVector NewLocation = FMath::VInterpTo(GetActorLocation(), TargetLocation, DeltaTime, LocationFollowSpeed);
    SetActorLocation(NewLocation);

    // 회전 동기화(타겟의 시선(ViewRotation)을 내 컨트롤러에 복사)
    if (AController* MyController = GetController())
    {
        FRotator TargetViewRotation = FRotator::ZeroRotator;
        bool bFoundRotation = false;

        if (APawn* TargetPawn = Cast<APawn>(Target))
        {
            // 타겟의 PlayerState에서 정확한 회전값 가져오기
            if (ASFPlayerState* TargetPS = TargetPawn->GetPlayerState<ASFPlayerState>())
            {
                TargetViewRotation = TargetPS->GetReplicatedViewRotation();
                bFoundRotation = true;
            }
            // PlayerState가 없는 경우(NPC 등)
            else 
            {
                TargetViewRotation = TargetPawn->GetBaseAimRotation();
                bFoundRotation = true;
            }
        }

        if (bFoundRotation)
        {
            FRotator CurrentRotation = MyController->GetControlRotation();
            FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetViewRotation, DeltaTime, RotationFollowSpeed);
            
            MyController->SetControlRotation(NewRotation);
        }
    }
    // 추후 오빗 카메라로 변경하려면 SetupPlayerInputComponent(마우스 입력) 구현 
}

void ASFSpectatorPawn::SetFollowTarget(AActor* InTarget)
{
    FollowTarget = InTarget;

    // 타겟 설정 시 즉시 위치/회전 스냅 (초기 점프 방지)
    if (InTarget)
    {
        SetActorLocation(InTarget->GetActorLocation());

        if (APawn* TargetPawn = Cast<APawn>(InTarget))
        {
            if (AController* MyController = GetController())
            {
                MyController->SetControlRotation(TargetPawn->GetBaseAimRotation());
            }
        }
    }
}

TSubclassOf<USFCameraMode> ASFSpectatorPawn::DetermineCameraMode()
{
    return DefaultCameraModeClass;
}
