// Fill out your copyright notice in the Description page of Project Settings.


#include "LA_BTDecorator_SupportCondition.h"
#include "Character/Ally/LA_AllyAIController.h"
#include "Character/Player/Component/LA_HealthComponent.h"
#include "Kismet/GameplayStatics.h"

bool ULA_BTDecorator_SupportCondition::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{

    if (ALA_AllyAIController* AIController = Cast<ALA_AllyAIController>(OwnerComp.GetAIOwner()))
    {
        if (AActor* Player = Cast<AActor>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)))
        {
            if (ULA_HealthComponent* Health = Cast<ULA_HealthComponent>(Player->FindComponentByClass<ULA_HealthComponent>()))
            {
                return Health->GetHealthPercent() <= 0.3f;
            }
        }
    }
    return false;
}
