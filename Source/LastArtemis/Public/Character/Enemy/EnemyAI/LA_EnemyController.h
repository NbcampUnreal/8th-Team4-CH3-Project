#pragma once

#include "CoreMinimal.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "LA_EnemyController.generated.h"

struct FAIStimulus;

UCLASS()
class LASTARTEMIS_API ALA_EnemyController : public AAIController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI | Perception", meta = (ClampMin = "0.0"))
	float SightRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI | Perception", meta = (ClampMin = "0.0"))
	float LoseSightRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI | Perception", meta = (ClampMin = "0.0"))
	float PeripheralVisionAngleDegrees;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class UAIPerceptionComponent* EnemyPerceptionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class UAISenseConfig_Sight* SightConfig;

	UFUNCTION()
	void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    UFUNCTION()
    void OnTargetDetected(AActor* Actor, FAIStimulus Stimulus);

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    class UBehaviorTree* BTAsset;

	// --- 상태 변수 ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	AActor* TargetPlayer;

	// 공격 범위
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float AttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float AttackDelay;

	FTimerHandle AttackTimerHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	bool bIsAttacking;

public:
	ALA_EnemyController();

	UFUNCTION(BlueprintCallable, Category = "AI")
	void ChasePlayer();

	UFUNCTION(BlueprintCallable, Category = "AI")
	void AttackPlayer();

	UFUNCTION()
	void ResetAttackState();
};
