// Fill out your copyright notice in the Description page of Project Settings.

#include "SFEnemyController.h"

#include "AI/StateMachine/SFStateMachine.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Perception/AISense_Sight.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "NavigationSystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SFEnemyController)


static TAutoConsoleVariable<bool> CVarShowAIDebug(
    TEXT("AI.ShowDebug"),
    false,
    TEXT("AI 감지 범위 및 시각화 디버그 표시\n")
    TEXT("0: 끔, 1: 켬"),
    ECVF_Default
);


ASFEnemyController::ASFEnemyController(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // AI Perception 생성
    AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
    SetPerceptionComponent(*AIPerception);

    // 시야 감각 설정 생성 및 기본값 적용
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = SightRadius;
    SightConfig->LoseSightRadius = LoseSightRadius;
    SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngleDegrees;

    // 감지할 대상 유형
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

    // 시야 감지 지속 시간
    SightConfig->SetMaxAge(5.0f);
    SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.f;

    // 시야 감각 설정
    AIPerception->ConfigureSense(*SightConfig);
    AIPerception->SetDominantSense(UAISense_Sight::StaticClass());

    // Tick 활성화 (디버그용) 추후 false 예정
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PostUpdateWork;

    bIsInCombat = false;
    TargetActor = nullptr;
}


void ASFEnemyController::PreInitializeComponents()
{
    Super::PreInitializeComponents();

    UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}


void ASFEnemyController::BeginPlay()
{
    UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, UGameFrameworkComponentManager::NAME_GameActorReady);
    Super::BeginPlay();

    // Perception 이벤트 바인딩
    if (AIPerception)
    {
        AIPerception->OnTargetPerceptionUpdated.AddDynamic(
            this, &ASFEnemyController::OnTargetPerceptionUpdated
        );
        UE_LOG(LogTemp, Log, TEXT("[SFEnemyAI] Perception 이벤트 바인딩 완료"));
    }

    // 디버그 - 현재 범위 출력
    UE_LOG(LogTemp, Warning, TEXT("[SFEnemyAI] 감지 범위 → 근접: %.0f | 경계: %.0f | 시야: %.0f | 상실: %.0f"),
        MeleeRange, GuardRange, SightRadius, LoseSightRadius);
}



void ASFEnemyController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);
    Super::EndPlay(EndPlayReason);
}



void ASFEnemyController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    if (!InPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SFEnemyAI] OnPossess 실패: Pawn 이 nullptr 입니다"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[SFEnemyAI] 제어한 Pawn: %s"), *InPawn->GetName());

    // 스폰 위치 저장 (귀환 행동 등에 사용 가능)
    SpawnLocation = InPawn->GetActorLocation();
    UE_LOG(LogTemp, Log, TEXT("[SFEnemyAI] Spawn 위치 저장: %s"), *SpawnLocation.ToString());

    // 상태 머신 이벤트 바인딩
    BindingStateMachine(InPawn);
}


void ASFEnemyController::OnUnPossess()
{
    UnBindingStateMachine();
    Super::OnUnPossess();
}


void ASFEnemyController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    DrawDebugPerception();
}


void ASFEnemyController::SetBehaviorTree(UBehaviorTree* NewBehaviorTree)
{
    if (!NewBehaviorTree || !CachedBehaviorTreeComponent)
        return;

    if (CachedBehaviorTreeComponent->GetCurrentTree() == NewBehaviorTree)
        return;

    RunBehaviorTree(NewBehaviorTree);
}

void ASFEnemyController::BindingStateMachine(const APawn* InPawn)
{
    if (!InPawn)
        return;

    CachedBehaviorTreeComponent = Cast<UBehaviorTreeComponent>(GetBrainComponent());
    CachedBlackboardComponent = GetBlackboardComponent();

    if (!CachedBehaviorTreeComponent || !CachedBlackboardComponent)
        return;

    USFStateMachine* StateMachine = USFStateMachine::FindStateMachineComponent(InPawn);
    if (StateMachine)
    {
        StateMachine->OnChangeTreeDelegate.AddUObject(this, &ThisClass::ChangeBehaviorTree);
        StateMachine->OnStopTreeDelegate.AddUObject(this, &ThisClass::StopBehaviorTree);
    }
}

void ASFEnemyController::UnBindingStateMachine()
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
        return;

    USFStateMachine* StateMachine = USFStateMachine::FindStateMachineComponent(ControlledPawn);
    if (StateMachine)
    {
        StateMachine->OnChangeTreeDelegate.RemoveAll(this);
        StateMachine->OnStopTreeDelegate.RemoveAll(this);
    }
}

void ASFEnemyController::StopBehaviorTree()
{
    if (CachedBehaviorTreeComponent)
        CachedBehaviorTreeComponent->StopTree();
}

void ASFEnemyController::ChangeBehaviorTree(FGameplayTag GameplayTag)
{
    if (!GameplayTag.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("[SFEnemyAI] ChangeBehaviorTree 실패: 태그가 유효하지 않음"));
        return;
    }

    UBehaviorTree* NewTree = BehaviorTreeContainer.GetBehaviourTree(GameplayTag);
    if (NewTree)
    {
        UE_LOG(LogTemp, Log, TEXT("[SFEnemyAI] BehaviorTree 변경: %s"), *GameplayTag.ToString());
        SetBehaviorTree(NewTree);
    }
}

bool ASFEnemyController::RunBehaviorTree(UBehaviorTree* BehaviorTree)
{
    if (!BehaviorTree)
    {
        UE_LOG(LogTemp, Error, TEXT("[SFEnemyAI] RunBehaviorTree 실패: 전달된 트리가 nullptr"));
        return false;
    }

    const bool bSuccess = Super::RunBehaviorTree(BehaviorTree);
    UE_LOG(LogTemp, Log, TEXT("[SFEnemyAI] BehaviorTree 실행: %s"), *BehaviorTree->GetName());
    bIsInCombat = false;
    return bSuccess;
}



void ASFEnemyController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor)
        return;

    // 시야 감각만 처리
    if (!Stimulus.Type.IsValid() || Stimulus.Type != UAISense::GetSenseID<UAISense_Sight>())
        return;

    // 특정 태그만 타겟으로 감지하려는 경우
    if (!TargetTag.IsNone() && !Actor->ActorHasTag(TargetTag))
        return;

    UE_LOG(LogTemp, Log, TEXT("[SFEnemyAI] 감지된 액터: %s (Tag: %s)"), *Actor->GetName(), *TargetTag.ToString());

    // 감지 성공 (전투 상태 진입)
    if (Stimulus.WasSuccessfullySensed())
    {
        TargetActor = Actor;

        if (!bIsInCombat)
        {
            bIsInCombat = true;

            // 전투 중에는 시야 360도
            SightConfig->PeripheralVisionAngleDegrees = 180.f;
            AIPerception->ConfigureSense(*SightConfig);

            UE_LOG(LogTemp, Warning, TEXT("[SFEnemyAI] 상태: Idle → Combat (타겟: %s) | 시야 90° → 360°"),
                *Actor->GetName());
        }

        SetFocus(Actor, EAIFocusPriority::Gameplay);

        if (CachedBlackboardComponent)
        {
            CachedBlackboardComponent->SetValueAsObject("TargetActor", Actor);
            CachedBlackboardComponent->SetValueAsBool("bHasTarget", true);
            CachedBlackboardComponent->SetValueAsBool("bIsInCombat", true);
            CachedBlackboardComponent->SetValueAsVector("LastKnownPosition", Stimulus.StimulusLocation);
        }
    }
    // 감지 상실 (전투 종료)
    else
    {
        if (bIsInCombat)
        {
            bIsInCombat = false;
            TargetActor = nullptr;

            // Idle 시야 각도 복원
            SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngleDegrees;
            AIPerception->ConfigureSense(*SightConfig);

            UE_LOG(LogTemp, Warning, TEXT("[SFEnemyAI] 상태: Combat → Idle | 시야 360° → 90°"));

            ClearFocus(EAIFocusPriority::Gameplay);

            if (CachedBlackboardComponent)
            {
                CachedBlackboardComponent->ClearValue("TargetActor");
                CachedBlackboardComponent->SetValueAsBool("bHasTarget", false);
                CachedBlackboardComponent->SetValueAsBool("bIsInCombat", false);
            }
        }
    }
}


float ASFEnemyController::GetDistanceToTarget() const
{
    if (!TargetActor || !GetPawn())
        return 0.f;

    return FVector::Dist(GetPawn()->GetActorLocation(), TargetActor->GetActorLocation());
}

bool ASFEnemyController::IsInMeleeRange() const
{
    const float Distance = GetDistanceToTarget();
    return Distance > 0.f && Distance <= MeleeRange;
}

bool ASFEnemyController::IsInGuardRange() const
{
    const float Distance = GetDistanceToTarget();
    return Distance > 0.f && Distance <= GuardRange;
}

bool ASFEnemyController::IsInTrackingRange() const
{
    const float Distance = GetDistanceToTarget();
    return Distance > 0.f && Distance <= LoseSightRadius;
}

AActor* ASFEnemyController::FindBestTarget()
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
        return nullptr;

    TArray<AActor*> PerceivedActors;
    if (AIPerception)
        AIPerception->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActors);

    if (PerceivedActors.Num() == 0)
        return nullptr;

    AActor* BestTarget = nullptr;
    float BestScore = -1.f;

    const FVector MyLocation = ControlledPawn->GetActorLocation();
    const FVector MyForward = ControlledPawn->GetActorForwardVector();

    for (AActor* Actor : PerceivedActors)
    {
        if (!TargetTag.IsNone() && !Actor->ActorHasTag(TargetTag))
            continue;

        float Score = 0.f;

        const float Distance = FVector::Dist(MyLocation, Actor->GetActorLocation());
        Score += FMath::Clamp(1000.f - (Distance / 10.f), 0.f, 1000.f);

        const FVector ToTarget = (Actor->GetActorLocation() - MyLocation).GetSafeNormal();
        const float Dot = FVector::DotProduct(MyForward, ToTarget);
        Score += FMath::Clamp((Dot + 1.f) * 250.f, 0.f, 500.f);

        UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
        if (NavSys)
        {
            FPathFindingQuery Query(this, *NavSys->GetDefaultNavDataInstance(), MyLocation, Actor->GetActorLocation());
            FPathFindingResult Result = NavSys->FindPathSync(Query);
            if (!Result.IsSuccessful() || !Result.Path.IsValid())
                Score *= 0.1f;
        }

        if (Actor == TargetActor)
            Score += EngagementBonus;

        if (Score > BestScore)
        {
            BestScore = Score;
            BestTarget = Actor;
        }
    }

    return BestTarget;
}




void ASFEnemyController::DrawDebugPerception()
{
#if !UE_BUILD_SHIPPING
    const bool bShowDebug = CVarShowAIDebug.GetValueOnGameThread();
    if (!bShowDebug)
        return;

    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
        return;

    const FVector PawnLocation = ControlledPawn->GetActorLocation();
    const FRotator PawnRotation = ControlledPawn->GetActorRotation();
    const FVector ForwardVector = PawnRotation.Vector();
    const float LifeTime = 0.f;

    FVector EyeLocation = PawnLocation;
    if (ACharacter* ControlledCharacter = Cast<ACharacter>(ControlledPawn))
        EyeLocation.Z += ControlledCharacter->BaseEyeHeight;
    else
        EyeLocation.Z += 90.f;

    // Idle 시 상태
    if (!bIsInCombat)
    {
        DrawDebugCone(
            GetWorld(),
            EyeLocation,
            ForwardVector,
            SightRadius,
            FMath::DegreesToRadians(PeripheralVisionAngleDegrees),
            FMath::DegreesToRadians(PeripheralVisionAngleDegrees),
            16,
            FColor::Green,
            false,
            LifeTime,
            0,
            2.0f
        );

        DrawDebugSphere(
            GetWorld(),
            PawnLocation,
            SightRadius,
            16,
            FColor::Green,
            false,
            LifeTime,
            0,
            1.0f
        );

        DrawDebugString(
            GetWorld(),
            PawnLocation + FVector(0, 0, 150),
            TEXT("STATE: IDLE"),
            nullptr,
            FColor::White,
            LifeTime,
            true,
            1.5f
        );
    }
    // 전투 상태
    else
    {
        DrawDebugCone(
            GetWorld(),
            EyeLocation,
            ForwardVector,
            SightRadius,
            FMath::DegreesToRadians(180.f),
            FMath::DegreesToRadians(180.f),
            16,
            FColor::Cyan,
            false,
            LifeTime,
            0,
            1.5f
        );

        DrawDebugSphere(GetWorld(), PawnLocation, MeleeRange, 12, FColor::Red, false, LifeTime, 0, 4.0f);
        DrawDebugSphere(GetWorld(), PawnLocation, GuardRange, 16, FColor::Yellow, false, LifeTime, 0, 2.5f);
        DrawDebugSphere(GetWorld(), PawnLocation, LoseSightRadius, 16, FColor::Orange, false, LifeTime, 0, 2.0f);

        if (TargetActor)
        {
            DrawDebugLine(
                GetWorld(),
                PawnLocation + FVector(0, 0, 50),
                TargetActor->GetActorLocation() + FVector(0, 0, 50),
                FColor::Cyan,
                false,
                LifeTime,
                0,
                2.0f
            );

            const float Distance = GetDistanceToTarget();
            DrawDebugString(
                GetWorld(),
                PawnLocation + FVector(0, 0, 150),
                FString::Printf(TEXT("Distance: %.0f"), Distance),
                nullptr,
                FColor::Yellow,
                LifeTime,
                true,
                1.2f
            );

            FString StateText;
            FColor StateColor;
            if (Distance <= MeleeRange)
            {
                StateText = TEXT("STATE: MELEE");
                StateColor = FColor::Red;
            }
            else if (Distance <= GuardRange)
            {
                StateText = TEXT("STATE: GUARD");
                StateColor = FColor::Yellow;
            }
            else
            {
                StateText = TEXT("STATE: CHASE");
                StateColor = FColor::Orange;
            }

            DrawDebugString(
                GetWorld(),
                PawnLocation + FVector(0, 0, 120),
                StateText,
                nullptr,
                StateColor,
                LifeTime,
                true,
                1.5f
            );
        }
    }
#endif
}
