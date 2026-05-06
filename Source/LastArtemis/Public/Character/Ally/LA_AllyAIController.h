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


 /**
 * AIPerceptionComponent에서 감지 이벤트 발생 시 자동으로 호출되는 콜백 함수
 * 감지된 액터가 BaseCharacter인지 확인 후 Team.Ally 태그가 없으면 적으로 판단하여 Blackboard에 기록
 *
 * @param Actor		감지된 액터
 * @param Stimulus	감지 정보 (감지 여부, 감지 위치 등)
 */
    UFUNCTION(BlueprintCallable)
    void OnTargetDetected(AActor* Actor, FAIStimulus Stimulus);

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* BehaviorTreeAsset;
};
