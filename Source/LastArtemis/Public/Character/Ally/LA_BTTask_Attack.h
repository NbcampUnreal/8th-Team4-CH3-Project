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

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

    virtual uint16 GetInstanceMemorySize() const override
    {
        return sizeof(FAttackMemory);
    }
public:
    // 공격 범위
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    float AttackRange = 500.f;
    // 공격력
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    float Damage = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    float AttackInterval = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    int32 TotalAmmo = 30;





};
