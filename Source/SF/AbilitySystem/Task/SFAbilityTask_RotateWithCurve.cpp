#include "SFAbilityTask_RotateWithCurve.h"
#include "GameFramework/Character.h"

USFAbilityTask_RotateWithCurve* USFAbilityTask_RotateWithCurve::CreateRotateWithCurveTask(
    UGameplayAbility* OwningAbility,
    FName TaskInstanceName,
    float InMontageDuration,
    float InTargetYaw,
    float InActualTurnYaw)
{
    USFAbilityTask_RotateWithCurve* Task = NewAbilityTask<USFAbilityTask_RotateWithCurve>(OwningAbility, TaskInstanceName);
    
    Task->MontageDuration = InMontageDuration;
    Task->TargetYaw = InTargetYaw;
    Task->ActualTurnYaw = InActualTurnYaw;
    Task->TimeElapsed = 0.f;
    Task->bIsFinished = false;
    
    return Task;
}

void USFAbilityTask_RotateWithCurve::Activate()
{
    Super::Activate();

    bIsFinished = false;
    TimeElapsed = 0.f;

    ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
    if (!Character)
    {
        UE_LOG(LogTemp, Error, TEXT("[RotateTask] No Character!"));
        EndTask();
        return;
    }

    InitialYaw = Character->GetActorRotation().Yaw;
    bTickingTask = true;

    UE_LOG(LogTemp, Warning, TEXT("[RotateTask] Start - Duration: %.2f, From: %.1f, To: %.1f"), 
        MontageDuration, InitialYaw, TargetYaw);
}

void USFAbilityTask_RotateWithCurve::TickTask(float DeltaTime)
{
    if (bIsFinished) return;
    Super::TickTask(DeltaTime);

    ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
    if (!Character)
    {
        bIsFinished = true;
        EndTask();
        return;
    }

    TimeElapsed += DeltaTime;
    
    // 몽타주 길이 기준으로 진행도 계산
    float Progress = FMath::Clamp(TimeElapsed / MontageDuration, 0.f, 1.f);
    
    // 부드러운 커브 (EaseInOut)
    float Alpha = FMath::SmoothStep(0.f, 1.f, Progress);
    
    // 목표 각도까지 보간
    float NewYaw = FMath::Lerp(InitialYaw, TargetYaw, Alpha);
    
    FRotator NewRot = Character->GetActorRotation();
    NewRot.Yaw = NewYaw;
    Character->SetActorRotation(NewRot);
    
    // 완료 체크
    if (Progress >= 1.0f)
    {
        // 최종 위치 정확히 맞추기
        FRotator FinalRot = Character->GetActorRotation();
        FinalRot.Yaw = TargetYaw;
        Character->SetActorRotation(FinalRot);

        bIsFinished = true;
        
        UE_LOG(LogTemp, Warning, TEXT("[RotateTask] Completed - Final Yaw: %.1f"), TargetYaw);
        
        if (ShouldBroadcastAbilityTaskDelegates())
        {
            OnCompleted.Broadcast();
        }
        EndTask();
    }
}

void USFAbilityTask_RotateWithCurve::OnDestroy(bool bInOwnerFinished)
{
    if (!bIsFinished && ShouldBroadcastAbilityTaskDelegates())
    {
        UE_LOG(LogTemp, Warning, TEXT("[RotateTask] Cancelled"));
        OnCancelled.Broadcast();
    }
    Super::OnDestroy(bInOwnerFinished);
}