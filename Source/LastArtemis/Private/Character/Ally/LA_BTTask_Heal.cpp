// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Ally/LA_BTTask_Heal.h"
#include "Character/Ally/LA_AllyAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Player/Component/LA_HealthComponent.h"

EBTNodeResult::Type ULA_BTTask_Heal::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Heal Task Called"));
    if (AActor* Player = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(FName("PlayerActor"))))
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Player Found"));
        if (ULA_HealthComponent* HealthComponent = Player->FindComponentByClass<ULA_HealthComponent>())
        {

            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("Before Heal HP: %f/%f"), HealthComponent->GetCurrentHealth(), HealthComponent->GetMaxHealth()));
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("HealthComponent Found"));
            HealthComponent->Heal(HealAmount);
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("After Heal HP: %f/%f"), HealthComponent->GetCurrentHealth(), HealthComponent->GetMaxHealth()));
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Heal Succeeded"));
            return EBTNodeResult::Succeeded;
        }
    }

    return EBTNodeResult::Failed;
}
