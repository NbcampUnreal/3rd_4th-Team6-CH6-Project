#include "SFLockOnComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blueprint/UserWidget.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "SF/Character/SFCharacterBase.h"
#include "SF/Character/SFCharacterGameplayTags.h" 
// 필요한 인터페이스 헤더 추가 (예: EnemyActorComponent 등)

USFLockOnComponent::USFLockOnComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	// 기본 태그 설정
	TargetTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Type.Enemy")));
}

void USFLockOnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 타겟이 없으면 로직 수행 안 함
	if (!CurrentTarget) return;

	// 1. 타겟 유효성 및 해제 조건 검사 (거리, 시야 유예 등)
	UpdateLogic_TargetValidation(DeltaTime);
	
	// 유효성 검사에서 해제되었다면 중단
	if (!CurrentTarget) return;

	// 2. 타겟 스위칭 입력 처리
	HandleTargetSwitching(DeltaTime);

	// 3. 카메라 회전 업데이트 (핵심: 마우스 무시 & Hard Lock)
	UpdateLogic_CameraRotation(DeltaTime);

	// 4. 캐릭터 회전 업데이트 (카메라와 동기화)
	UpdateLogic_CharacterRotation(DeltaTime);

	// 5. 위젯 위치 업데이트 (필요 시)
	UpdateLogic_WidgetPosition(DeltaTime);
}

// =========================================================
//  Logic Implementations
// =========================================================

void USFLockOnComponent::UpdateLogic_TargetValidation(float DeltaTime)
{
	bool bShouldBreak = false;

	// A. 기본 검사 (사망, 너무 먼 거리)
	float Distance = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
	if (!IsTargetValidBasic(CurrentTarget) || Distance > LockOnBreakDistance)
	{
		bShouldBreak = true;
	}
	else
	{
		// B. 시야 가림 유예 (Grace Period) 검사
		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(GetOwner());
		QueryParams.AddIgnoredActor(CurrentTarget);

		// 발 밑(Root)보다는 약간 위(허리/가슴) 높이에서 레이를 쏘는 것이 정확함
		FVector Start = GetOwner()->GetActorLocation() + FVector(0, 0, 50);
		FVector End = CurrentTarget->GetActorLocation() + FVector(0, 0, 50);

		bool bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult, Start, End, ECC_Visibility, QueryParams
		);

		if (bHit)
		{
			// 장애물에 가려짐 -> 타이머 누적
			TimeSinceTargetHidden += DeltaTime;
			if (TimeSinceTargetHidden > LostTargetMemoryTime)
			{
				// 유예 시간 초과 시 락온 해제
				bShouldBreak = true;
			}
		}
		else
		{
			// 시야 확보됨 -> 타이머 초기화
			TimeSinceTargetHidden = 0.0f;
		}
	}

	if (bShouldBreak)
	{
		EndLockOn();
	}
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

	// 1. 입력 감지 (패드 & 마우스 통합)
	float InputX = 0.0f;
	float InputY = 0.0f;

	// [A] 게임패드 스틱
	PC->GetInputAnalogStickState(EControllerAnalogStick::CAS_RightStick, InputX, InputY);

	// [B] 마우스
	if (FMath::IsNearlyZero(InputX) && FMath::IsNearlyZero(InputY))
	{
		float MouseX = 0.0f, MouseY = 0.0f;
		PC->GetInputMouseDelta(MouseX, MouseY);
		
		// 마우스 감도 보정 (너무 민감하지 않게)
		float MouseSensitivity = 0.3f; 
		InputX = MouseX * MouseSensitivity;
		InputY = MouseY * MouseSensitivity;
	}

	FVector2D CurrentInput(InputX, InputY);

	// 2. 입력 크기 체크 (Threshold)
	if (CurrentInput.Size() < SwitchInputThreshold) return;

	// 3. 스위칭 대상 탐색
	FRotator CamRot = PC->PlayerCameraManager->GetCameraRotation();
	FVector CamRight = CamRot.RotateVector(FVector::RightVector);
	FVector CamUp = CamRot.RotateVector(FVector::UpVector);
	
	FVector SearchDirection = (CamRight * CurrentInput.X + CamUp * CurrentInput.Y).GetSafeNormal();
	
	TArray<AActor*> OverlappedActors;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	UKismetSystemLibrary::SphereOverlapActors(
		this, OwnerPawn->GetActorLocation(), LockOnDistance, ObjectTypes,
		ASFCharacterBase::StaticClass(), { OwnerPawn, CurrentTarget }, OverlappedActors
	);

	AActor* BestNewTarget = nullptr;
	float ClosestDistSq = FLT_MAX; 

	for (AActor* Candidate : OverlappedActors)
	{
		if (!IsTargetValidBasic(Candidate)) continue;

		// 현재 타겟에서 후보로 향하는 방향 벡터
		FVector FromTargetToCand = Candidate->GetActorLocation() - CurrentTarget->GetActorLocation();
		float DistSq = FromTargetToCand.SizeSquared(); // 거리의 제곱 (비교용)
		FVector DirToCand = FromTargetToCand.GetSafeNormal();

		// 입력한 방향(SearchDirection)과 후보가 있는 방향(DirToCand)이 얼마나 일치하는지?
		float InputDot = FVector::DotProduct(SearchDirection, DirToCand);

		if (InputDot > SwitchAngularLimit) 
		{
			if (DistSq < ClosestDistSq)
			{
				ClosestDistSq = DistSq;
				BestNewTarget = Candidate;
			}
		}
	}

	if (BestNewTarget)
	{
		CurrentTarget = BestNewTarget;
		CurrentSwitchCooldown = SwitchCooldown;
		TimeSinceTargetHidden = 0.0f; // 스위칭 시 시야 타이머 초기화

		DestroyLockOnWidget();
		CreateLockOnWidget();

		// [핵심] 스위칭 모드 활성화 (카메라를 부드럽게 이동시키기 위함)
		bIsSwitchingTarget = true;
	}
}

void USFLockOnComponent::UpdateLogic_CameraRotation(float DeltaTime)
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;
	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PC) return;

	// 1. 타겟 목표 위치 계산 (소켓 우선)
	FVector TargetLoc = CurrentTarget->GetActorLocation();
	if (USceneComponent* TargetMesh = CurrentTarget->FindComponentByClass<USceneComponent>())
	{
		if (TargetMesh->DoesSocketExist(LockOnSocketName))
		{
			TargetLoc = TargetMesh->GetSocketLocation(LockOnSocketName);
		}
		else
		{
			TargetLoc.Z += 50.0f; // 소켓 없으면 약간 위 조준
		}
	}

	// 2. 목표 회전값 계산 (현재 카메라 위치 기준)
	FVector CameraLoc = PC->PlayerCameraManager->GetCameraLocation();
	FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(CameraLoc, TargetLoc);
	LookAtRot.Pitch = FMath::Clamp(LookAtRot.Pitch, -45.0f, 45.0f);

	// 3. 상황에 따른 보간 속도 결정
	float InterpSpeed = 30.0f; // 기본: 아주 빠름 (Hard Lock - 마우스 고정 효과)

	if (bIsSwitchingTarget)
	{
		InterpSpeed = TargetSwitchInterpSpeed; // 스위칭 중: 부드럽게 (Slow)

		// 목표에 거의 도달했는지 확인 (완료 시 Hard Lock 복귀)
		FRotator Delta = (LookAtRot - LastLockOnRotation).GetNormalized();
		if (FMath::Abs(Delta.Yaw) < 2.0f && FMath::Abs(Delta.Pitch) < 2.0f)
		{
			bIsSwitchingTarget = false;
		}
	}

	// 4. 회전 적용 (마우스 입력 무시를 위해 LastLockOnRotation 기준 보간)
	FRotator SmoothRot = FMath::RInterpTo(LastLockOnRotation, LookAtRot, DeltaTime, InterpSpeed);
	
	PC->SetControlRotation(SmoothRot);
	LastLockOnRotation = SmoothRot; // 다음 프레임을 위해 저장
}

void USFLockOnComponent::UpdateLogic_CharacterRotation(float DeltaTime)
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;

	ACharacter* Character = Cast<ACharacter>(OwnerPawn);
	if (!Character) return;

	bool bIsSprinting = false;

	// 1. 달리기 태그 확인 (GAS)
	if (const IGameplayTagAssetInterface* TagInterface = Cast<const IGameplayTagAssetInterface>(Character))
	{
		// SprintTag가 할당되어 있고, 캐릭터가 그 태그를 가지고 있다면
		if (SprintTag.IsValid() && TagInterface->HasMatchingGameplayTag(SprintTag))
		{
			bIsSprinting = true;
		}
	}

	// 2. 상태에 따른 회전 로직 분기
	if (bIsSprinting)
	{
		// [달리기 중]: 락온 고정 해제 -> 이동 방향을 바라봄 (Free Movement)
		Character->GetCharacterMovement()->bOrientRotationToMovement = true;
	}
	else
	{
		// [일반 락온]: 이동 방향 회전 끄기 -> 타겟 방향 강제 고정 (Strafing)
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;

		// 카메라(컨트롤러)가 바라보는 방향(=타겟 방향)으로 캐릭터 몸통 정렬
		FRotator TargetRot = FRotator(0.0f, LastLockOnRotation.Yaw, 0.0f);
        
		// 부드럽게 회전
		FRotator CurrentRot = Character->GetActorRotation();
		FRotator SmoothRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, 15.0f);
        
		Character->SetActorRotation(SmoothRot);
	}
}

void USFLockOnComponent::UpdateLogic_WidgetPosition(float DeltaTime)
{
	// 위젯 위치 업데이트는 보통 Widget BP의 Tick에서 ProjectWorldToScreen을 사용하는 것이 효율적입니다.
	// 필요하다면 여기서 구현 가능합니다.
}

// =========================================================
//  Main Functions
// =========================================================

bool USFLockOnComponent::TryLockOn()
{
	// 1. 쿨타임 체크 (연타 방지)
	double CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastLockOnToggleTime < 0.2)
	{
		return true; // 쿨타임 중엔 처리된 것으로 간주 (리셋 방지)
	}

	// 2. 이미 락온 중 -> 해제 (Toggle Off)
	if (CurrentTarget)
	{
		EndLockOn();
		LastLockOnToggleTime = CurrentTime;
		return true; // "해제 성공"도 true 반환
	}

	// 3. 새로운 타겟 탐색
	AActor* NewTarget = FindBestTarget();
	if (NewTarget)
	{
		CurrentTarget = NewTarget;
		TimeSinceTargetHidden = 0.0f;

		// GAS 태그 적용 및 캐릭터 상태 변경
		if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
		{
			if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwnerPawn))
			{
				ASC->AddLooseGameplayTag(SFGameplayTags::Character_State_LockedOn);
			}

			if (ACharacter* Character = Cast<ACharacter>(OwnerPawn))
			{
				// 마우스 입력이 캐릭터 회전에 영향 주지 않도록 연결 끊기
				Character->bUseControllerRotationYaw = false;
				Character->GetCharacterMovement()->bOrientRotationToMovement = false;
				Character->GetCharacterMovement()->MaxWalkSpeed *= 0.8f; // 이동 속도 감소 (Strafing)
			}

			// 초기화: 현재 뷰를 시작점으로 설정
			if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
			{
				LastLockOnRotation = PC->GetControlRotation();
			}
			bIsSwitchingTarget = false;
		}

		CreateLockOnWidget();
		LastLockOnToggleTime = CurrentTime;
		return true; // 락온 성공
	}

	// 4. 타겟 없음 -> 카메라 리셋 필요 (false 반환)
	return false;
}

void USFLockOnComponent::EndLockOn()
{
	DestroyLockOnWidget();
	CurrentTarget = nullptr;
	TimeSinceTargetHidden = 0.0f;

	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		// GAS 태그 제거
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwnerPawn))
		{
			ASC->RemoveLooseGameplayTag(SFGameplayTags::Character_State_LockedOn);
		}

		// 캐릭터 상태 복구
		if (ACharacter* Character = Cast<ACharacter>(OwnerPawn))
		{
			Character->bUseControllerRotationYaw = false;
			Character->GetCharacterMovement()->bOrientRotationToMovement = true;
			Character->GetCharacterMovement()->MaxWalkSpeed /= 0.8f;
		}
	}
}

// =========================================================
//  Helper Functions
// =========================================================

AActor* USFLockOnComponent::FindBestTarget()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return nullptr;
	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PC) return nullptr;

	FVector CameraLoc;
	FRotator CameraRot;
	PC->GetPlayerViewPoint(CameraLoc, CameraRot);
	FVector CameraForward = CameraRot.Vector();

	TArray<AActor*> OverlappedActors;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	UKismetSystemLibrary::SphereOverlapActors(
		this, OwnerPawn->GetActorLocation(), LockOnDistance, ObjectTypes,
		ASFCharacterBase::StaticClass(), { OwnerPawn }, OverlappedActors
	);

	AActor* BestTarget = nullptr;
	float BestScore = -1.0f;

	for (AActor* Actor : OverlappedActors)
	{
		if (!IsTargetValidBasic(Actor)) continue;

		FVector DirToTarget = (Actor->GetActorLocation() - CameraLoc).GetSafeNormal();
		float DotResult = FVector::DotProduct(CameraForward, DirToTarget);

		// 화면 중앙 범위 밖이면 제외
		if (DotResult < ScreenCenterWeight) continue;

		float Distance = FVector::Dist(OwnerPawn->GetActorLocation(), Actor->GetActorLocation());
		float DistanceScore = 1.0f - (Distance / LockOnDistance);

		// 중앙 정렬 우선 점수 계산
		float FinalScore = (DotResult * 2.0f) + DistanceScore;

		if (FinalScore > BestScore)
		{
			BestScore = FinalScore;
			BestTarget = Actor;
		}
	}

	return BestTarget;
}

bool USFLockOnComponent::IsTargetValidBasic(AActor* TargetActor) const
{
	if (!TargetActor || !TargetActor->IsValidLowLevel()) return false;
	
	if (const IGameplayTagAssetInterface* TagInterface = Cast<const IGameplayTagAssetInterface>(TargetActor))
	{
		if (TagInterface->HasMatchingGameplayTag(SFGameplayTags::Character_State_Dead))
		{
			return false; // 이미 사망한 타겟은 무시
		}
	}
	
	return true;
}

void USFLockOnComponent::CreateLockOnWidget()
{
	if (!LockOnWidgetClass || !GetWorld()) return;

	if (!LockOnWidgetInstance)
	{
		LockOnWidgetInstance = CreateWidget<UUserWidget>(GetWorld(), LockOnWidgetClass);
	}

	if (LockOnWidgetInstance && !LockOnWidgetInstance->IsInViewport())
	{
		LockOnWidgetInstance->AddToViewport(-1);
	}
}

void USFLockOnComponent::DestroyLockOnWidget()
{
	if (LockOnWidgetInstance)
	{
		LockOnWidgetInstance->RemoveFromParent();
		LockOnWidgetInstance = nullptr;
	}
}