#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "LA_BTTask_Attack.generated.h"

struct FAttackMemory
{
    float ElapsedTime = 0.f;    // 연사 타이머
    int32 CurrentAmmo = 30;     // 남은 총알
    float ReloadTimer = 0.f;    // 재장전 진행 타이머
    bool bIsReloading = false;    // 재장전 진행 여부
};

UCLASS()
class LASTARTEMIS_API ULA_BTTask_Attack : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

protected:
    ULA_BTTask_Attack();

    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

public:


    // 공격 범위
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    float AttackRange;
    // 공격력
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    float Damage;
    // 공격 시간 간격
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    float AttackInterval;
    // 최대 탄약 수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    int32 TotalAmmo;


    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual uint16 GetInstanceMemorySize() const override
    {
        return sizeof(FAttackMemory);
    }

};
