#include "SFLockOnComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "SF/Character/SFCharacterBase.h" 
#include "SF/Interface/EnemyActorComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "SF/Character/SFCharacterGameplayTags.h" 

USFLockOnComponent::USFLockOnComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	
	// 기본값 설정
	TargetTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Type.Enemy"))); 
}

void USFLockOnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentTarget)
	{
		if (!IsTargetValid(CurrentTarget))
		{
			EndLockOn();
		}
		else
		{
			APawn* OwnerPawn = Cast<APawn>(GetOwner());
			if (OwnerPawn)
			{
				if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
				{
					// 1. 타겟 스위칭 입력 처리
					HandleTargetSwitching(DeltaTime);

					// 2. 타겟 위치 가져오기
					FVector TargetLoc = CurrentTarget->GetActorLocation();
					if (USceneComponent* TargetMesh = CurrentTarget->FindComponentByClass<USceneComponent>())
					{
						if (TargetMesh->DoesSocketExist(LockOnSocketName))
						{
							TargetLoc = TargetMesh->GetSocketLocation(LockOnSocketName);
						}
						else
						{
							TargetLoc.Z += 50.0f; 
						}
					}

					// 3. 목표 회전값 계산 (마우스 입력 무시하고, 코드 기반으로 계산)
					FVector CameraLoc = PC->PlayerCameraManager->GetCameraLocation();
					FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(CameraLoc, TargetLoc);
					LookAtRot.Pitch = FMath::Clamp(LookAtRot.Pitch, -45.0f, 45.0f);

					// 4. 상황에 따른 가변 속도 적용
					float InterpSpeed = 30.0f; // 기본: 아주 빠름 (Hard Lock 느낌)

					if (bIsSwitchingTarget)
					{
						InterpSpeed = 8.0f; // 스위칭 중: 부드럽게 이동 (Slow)
						
						// 목표에 거의 도달했는지 확인 (Yaw 차이가 2도 미만이면 스위칭 종료)
						FRotator Delta = (LookAtRot - LastLockOnRotation).GetNormalized();
						if (FMath::Abs(Delta.Yaw) < 2.0f && FMath::Abs(Delta.Pitch) < 2.0f)
						{
							bIsSwitchingTarget = false; // 다시 Hard Lock 모드로 전환
						}
					}

					// 5. 카메라 회전 적용 (마우스 무시하고 부드러운 값 적용)
					FRotator SmoothRot = FMath::RInterpTo(LastLockOnRotation, LookAtRot, DeltaTime, InterpSpeed);
					PC->SetControlRotation(SmoothRot);
					LastLockOnRotation = SmoothRot;

					// 6. 캐릭터 몸통 강제 회전
					FRotator CharacterRot = FRotator(0.0f, SmoothRot.Yaw, 0.0f); // Pitch/Roll은 0으로
					OwnerPawn->SetActorRotation(CharacterRot);
				}
			}
		}
	}
}

bool USFLockOnComponent::TryLockOn()
{
	// 1. 쿨타임 체크 (키 홀드 시 무한 반복 방지)
	double CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastLockOnToggleTime < 0.2) // 0.2초 쿨타임
	{
		return true; // 쿨타임 중이면 "성공(처리됨)"으로 간주해 리셋 방지
	}

	// 2. 이미 락온 중이라면 해제 (Toggle)
	if (CurrentTarget)
	{
		EndLockOn();
		LastLockOnToggleTime = CurrentTime; // 시간 갱신
		return true; // [중요] "해제 성공"도 true로 반환 (카메라 리셋 안 되게)
	}

	// 3. 새로운 타겟 탐색
	AActor* NewTarget = FindBestTarget();
	if (NewTarget)
	{
		CurrentTarget = NewTarget;
		
		// 락온 상태 태그 적용
		APawn* OwnerPawn = Cast<APawn>(GetOwner());
		if (OwnerPawn)
		{
			if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwnerPawn))
			{
				ASC->AddLooseGameplayTag(SFGameplayTags::Character_State_LockedOn);
			}

			// 캐릭터 이동 모드 변경 (Strafe)
			if (ACharacter* Character = Cast<ACharacter>(OwnerPawn))
			{
				Character->bUseControllerRotationYaw = false; // 컨트롤러 회전 고정
				Character->GetCharacterMovement()->bOrientRotationToMovement = false; // 이동 방향으로 회전 끄기
				Character->GetCharacterMovement()->MaxWalkSpeed *= 0.8f; // 락온 시 이동 속도 살짝 감소
			}
			
			// 락온 시작 시 현재 컨트롤러 회전값을 저장 (초기화)
			if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
			{
				LastLockOnRotation = PC->GetControlRotation();
			}
			bIsSwitchingTarget = false;
		}
		
		// TODO: 락온 성공 사운드 재생
		CreateLockOnWidget();
		LastLockOnToggleTime = CurrentTime; // 시간 갱신
		return true; // 락온 성공
	}

	// 4. 타겟 없음 (이때만 false 반환하여 카메라 리셋 유도)
	return false;
}

void USFLockOnComponent::EndLockOn()
{
	DestroyLockOnWidget();
	
	CurrentTarget = nullptr;

	// 락온 상태 태그 제거
	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwnerPawn))
		{
			ASC->RemoveLooseGameplayTag(SFGameplayTags::Character_State_LockedOn);
		}

		if (ACharacter* Character = Cast<ACharacter>(OwnerPawn))
		{
			Character->bUseControllerRotationYaw = false; 
			Character->GetCharacterMovement()->bOrientRotationToMovement = true; 
			Character->GetCharacterMovement()->MaxWalkSpeed /= 0.8f; 
		}
	}

	// TODO: 락온 해제 사운드
}

AActor* USFLockOnComponent::FindBestTarget()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return nullptr;

	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PC) return nullptr;

	FVector CameraLocation;
	FRotator CameraRotation;
	PC->GetPlayerViewPoint(CameraLocation, CameraRotation);
	FVector CameraForward = CameraRotation.Vector();

	// 1. 구체 충돌로 주변 액터 수집
	TArray<AActor*> OverlappedActors;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn)); // Pawn 채널 검색

	UKismetSystemLibrary::SphereOverlapActors(
		this,
		OwnerPawn->GetActorLocation(),
		LockOnDistance,
		ObjectTypes,
		ASFCharacterBase::StaticClass(), // 검색 클래스 필터
		{ OwnerPawn }, // 자기 자신 제외
		OverlappedActors
	);

	AActor* BestTarget = nullptr;
	float BestScore = -1.0f; // 점수가 높을수록 좋음

	for (AActor* Actor : OverlappedActors)
	{
		if (!IsTargetValid(Actor)) continue;

		// 2. 점수 계산 (Dot Product)
		FVector DirectionToTarget = (Actor->GetActorLocation() - CameraLocation).GetSafeNormal();
		float DotResult = FVector::DotProduct(CameraForward, DirectionToTarget);
		
		// 화면 뒤에 있거나 시야각을 벗어나면 제외 (예: 45도 = 0.707)
		if (DotResult < ScreenCenterWeight) continue;

		// 거리 가중치 (가까울수록 점수 +)
		float Distance = FVector::Dist(OwnerPawn->GetActorLocation(), Actor->GetActorLocation());
		float DistanceScore = 1.0f - (Distance / LockOnDistance); // 0~1

		// 최종 점수: 내적(중앙 정렬) 비중을 높게 설정
		float FinalScore = (DotResult * 2.0f) + DistanceScore;

		if (FinalScore > BestScore)
		{
			BestScore = FinalScore;
			BestTarget = Actor;
		}
	}

	return BestTarget;
}

bool USFLockOnComponent::IsTargetValid(AActor* TargetActor) const
{
	if (!TargetActor || !TargetActor->IsValidLowLevel()) return false;

	// 1. 사망 여부 확인 (인터페이스나 태그 사용)
	// 예시: if (TargetActor->Implements<USFHealthInterface>() && ISFHealthInterface::Execute_IsDead(TargetActor)) return false;
	
	// 2. 거리 확인
	float Distance = FVector::Dist(GetOwner()->GetActorLocation(), TargetActor->GetActorLocation());
	if (Distance > LockOnBreakDistance) return false;

	// 3. 시야(LineTrace) 확인 - 장애물에 가려졌는지
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	QueryParams.AddIgnoredActor(TargetActor);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		GetOwner()->GetActorLocation() + FVector(0,0,50), // 발 밑 말고 약간 위에서 시작
		TargetActor->GetActorLocation(),
		ECC_Visibility, // 시야 채널
		QueryParams
	);

	// 무언가에 막혔다면 타겟 아님 (잠깐 가려지는 허용 시간 로직은 추후 추가)
	if (bHit) return false;

	return true;
}

void USFLockOnComponent::HandleTargetSwitching(float DeltaTime)
{
    if (CurrentSwitchCooldown > 0.0f)
    {
        CurrentSwitchCooldown -= DeltaTime;
        return;
    }

    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn) return;
    APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
    if (!PC) return;

    float InputX = 0.0f;
    float InputY = 0.0f;

    // 패드 & 마우스 입력 통합
    PC->GetInputAnalogStickState(EControllerAnalogStick::CAS_RightStick, InputX, InputY);
    if (FMath::IsNearlyZero(InputX) && FMath::IsNearlyZero(InputY))
    {
        float MouseX = 0.0f, MouseY = 0.0f;
        PC->GetInputMouseDelta(MouseX, MouseY);
        float MouseSensitivity = 0.3f; 
        InputX = MouseX * MouseSensitivity;
        InputY = MouseY * MouseSensitivity;
    }

    FVector2D CurrentInput(InputX, InputY);
    if (CurrentInput.Size() < SwitchInputThreshold) return;

    // --- 타겟 탐색 ---
    FVector CameraRight = PC->PlayerCameraManager->GetCameraRotation().RotateVector(FVector::RightVector);
    FVector SearchDirection = (CameraRight * CurrentInput.X).GetSafeNormal(); 

    TArray<AActor*> OverlappedActors;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
    
    UKismetSystemLibrary::SphereOverlapActors(
        this, OwnerPawn->GetActorLocation(), LockOnDistance, ObjectTypes, 
        ASFCharacterBase::StaticClass(), {OwnerPawn, CurrentTarget}, OverlappedActors
    );

    AActor* BestNewTarget = nullptr;
    float BestDot = 0.5f; 

    for (AActor* Candidate : OverlappedActors)
    {
        if (!IsTargetValid(Candidate)) continue;

        FVector ToCandidate = (Candidate->GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal();
        float InputDot = FVector::DotProduct(SearchDirection, ToCandidate);

        if (InputDot > BestDot) 
        {
            BestDot = InputDot;
            BestNewTarget = Candidate;
        }
    }

    if (BestNewTarget)
    {
        CurrentTarget = BestNewTarget;
        CurrentSwitchCooldown = SwitchCooldown;
        
        DestroyLockOnWidget(); 
        CreateLockOnWidget();
        
        // [핵심] 타겟이 바뀌었으므로 "스위칭 모드" 활성화 -> 부드럽게 이동 시작
        bIsSwitchingTarget = true; 
    }
}

// 위젯 생성
void USFLockOnComponent::CreateLockOnWidget()
{
    if (!LockOnWidgetClass || !GetWorld()) return;

    if (!LockOnWidgetInstance)
    {
        LockOnWidgetInstance = CreateWidget<UUserWidget>(GetWorld(), LockOnWidgetClass);
    }

    if (LockOnWidgetInstance)
    {
        if (!LockOnWidgetInstance->IsInViewport())
        {
            LockOnWidgetInstance->AddToViewport(-1); // Z-Order 뒤로
        }
        
        // 블루프린트에서 위젯 위치 업데이트를 위해 타겟 정보를 넘겨주는 로직이 필요하다면 여기서 호출
        // 예: LockOnWidgetInstance->SetTarget(CurrentTarget); (BP에서 함수 만들어야 함)
    }
}

// [추가] 위젯 제거
void USFLockOnComponent::DestroyLockOnWidget()
{
    if (LockOnWidgetInstance)
    {
        LockOnWidgetInstance->RemoveFromParent();
        LockOnWidgetInstance = nullptr; 
    }
}