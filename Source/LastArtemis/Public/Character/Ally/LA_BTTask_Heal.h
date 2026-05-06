#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "LA_BTTask_Heal.generated.h"



UCLASS()
class LASTARTEMIS_API ULA_BTTask_Heal : public UBTTaskNode
{
	GENERATED_BODY()

protected:
    /**
    * BehaviorTree에서 해당 Task가 실행될 때 자동으로 호출되는 함수
    * Blakcboard에서 PlayerActor를 가져와 HealthComponent를 통해 힐을 실행
    *
    * @param OwnerComp  이 Task를 실행하는 BehaviorTreeComponent
    * @param NodeMemory 노드별 메모리 블룩 (인스턴스별 데이터 저장용)
    * @return           태스크 성공 시 Succeded, HealthComponent를 찾지 못할 시 Failed 반환 
    */
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heal")
    float HealAmount = 50.f;
};
