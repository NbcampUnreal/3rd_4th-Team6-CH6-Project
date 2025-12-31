#pragma once
#include "CoreMinimal.h"
#include "DetourCrowdAIController.h"
#include "GameplayTagContainer.h"
#include "Character/Enemy/SFEnemyData.h"
#include "Interface/SFAIControllerInterface.h"
#include "SFBaseAIController.generated.h"

class USFEnemyCombatComponent;
class UBehaviorTreeComponent;
class UBlackboardComponent;
class UBehaviorTree;


UENUM(BlueprintType)
enum class EAIRotationMode : uint8
{
	None, // 회전 없음
	MovementDirection,    // 비전투 이동 (이동 방향으로 회전)
	ControllerYaw        // 전투 모드 (Controller 방향으로 회전)
};

UCLASS(Abstract)
class SF_API ASFBaseAIController  : public ADetourCrowdAIController, public ISFAIControllerInterface
{
	GENERATED_BODY()

public:
	ASFBaseAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/* ISFAIControllerInterface */
	virtual void InitializeAIController() override;
	virtual USFCombatComponentBase* GetCombatComponent() const override;
	virtual void Tick(float DeltaSeconds) override;
protected:
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#pragma region BehaviorTree
protected:
	void ChangeBehaviorTree(FGameplayTag GameplayTag);
	void StopBehaviorTree();
	void SetBehaviorTree(UBehaviorTree* BehaviorTree);
	void BindingStateMachine(const APawn* InPawn);
	void UnBindingStateMachine();

	UPROPERTY()
	FSFBehaviourWrapperContainer BehaviorTreeContainer;

	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> CachedBehaviorTreeComponent;

	UPROPERTY()
	TObjectPtr<UBlackboardComponent> CachedBlackboardComponent;

	UPROPERTY(Replicated, VisibleAnywhere, Category="AI|State")
	bool bIsInCombat = false;

	virtual bool RunBehaviorTree(UBehaviorTree* BehaviorTree) override;
#pragma endregion

#pragma region StateReaction
protected:
	UFUNCTION()
	void ReceiveStateStart(FGameplayTag StateTag);

	UFUNCTION()
	void ReceiveStateEnd(FGameplayTag StateTag);
#pragma endregion

#pragma region Combat
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI|Combat")
	TObjectPtr<AActor> TargetActor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI|Combat")
	TObjectPtr<USFCombatComponentBase> CombatComponent;
#pragma endregion

#pragma region Team
public:
	virtual void SetGenericTeamId(const FGenericTeamId& InTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual bool ShouldRotateActorByController() const;
	void SetbSuppressControlRotationUpdates(bool bInSuppress) { bSuppressControlRotationUpdates = bInSuppress; }
protected:
	UPROPERTY(Replicated)
	FGenericTeamId TeamId;

	bool bSuppressControlRotationUpdates = false;
#pragma endregion

#pragma region Rotation
protected:
	UPROPERTY()
	EAIRotationMode CurrentRotationMode = EAIRotationMode::ControllerYaw;

	virtual void UpdateControlRotation(float DeltaTime, bool bUpdatePawn) override;

public:
	void SetRotationMode(EAIRotationMode NewMode);
	EAIRotationMode GetCurrentRotationMode() const { return CurrentRotationMode; }
	
	virtual bool IsTurningInPlace() const { return false; }
	
	virtual float GetTurnThreshold() const { return 45.0f; }

protected:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|Rotation", meta=(ClampMin="0.0", ClampMax="1080.0"))
	float RotationInterpSpeed = 8.f;  // 회전 보간 속도
#pragma endregion
};
