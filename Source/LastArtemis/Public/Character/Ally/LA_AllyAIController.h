#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "LA_AllyAIController.generated.h"



UCLASS()
class LASTARTEMIS_API ALA_AllyAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	
	ALA_AllyAIController();

private:
	virtual void BeginPlay() override;

    UPROPERTY()
    UAISenseConfig_Sight* SightConfig;

    UFUNCTION(BlueprintCallable)
    void OnTargetDetected(AActor* Actor, FAIStimulus Stimulus);

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* BehaviorTreeAsset;
};
