#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "LA_EnemyController.generated.h"

UCLASS()
class LASTARTEMIS_API ALA_EnemyController : public AAIController
{
    GENERATED_BODY()

public:
    void BeginPlay();
    ALA_EnemyController();

protected:
    // --- 초기화 로직 ---
    virtual void OnPossess(APawn* InPawn) override;

    // --- AI Perception (눈) ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* EnemyPerceptionComponent;

    class UAISenseConfig_Sight* SightConfig;

    // 시야 감지 시 호출될 함수
    UFUNCTION()
    void OnTargetDetected(AActor* Actor, FAIStimulus Stimulus);

    // --- 비헤이비어 트리 에셋 ---
    UPROPERTY(EditDefaultsOnly, Category = "AI")
    class UBehaviorTree* BTAsset;

    // 시야 설정값 (생성자에서 기본값 설정 후 블루프린트에서 수정 가능)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI | Perception")
    float SightRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI | Perception")
    float LoseSightRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI | Perception")
    float PeripheralVisionAngleDegrees = 60.0f;
};
