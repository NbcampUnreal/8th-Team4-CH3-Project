// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Ally/LA_BTTask_Heal.h"
#include "Character/Ally/LA_AllyAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Player/Component/LA_HealthComponent.h"

EBTNodeResult::Type ULA_BTTask_Heal::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AActor* Player = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(FName("PlayerActor")));

    if (Player)
    {
        ULA_HealthComponent* HealthComponent = Player->FindComponentByClass<ULA_HealthComponent>();
        if (HealthComponent)
        {
            HealthComponent->Heal(HealAmount);
            return EBTNodeResult::Succeeded;
        }
    }
    return EBTNodeResult::Failed;
}
