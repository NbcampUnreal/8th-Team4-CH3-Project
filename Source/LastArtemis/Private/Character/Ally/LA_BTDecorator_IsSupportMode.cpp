// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Ally/LA_BTDecorator_IsSupportMode.h"
#include "BehaviorTree/BlackboardComponent.h"

ULA_BTDecorator_IsSupportMode::ULA_BTDecorator_IsSupportMode()
{
    NodeName = TEXT("Is Support Mode");
}

bool ULA_BTDecorator_IsSupportMode::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory) const
{
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (!Blackboard) return false;

    return Blackboard ->GetValueAsBool(FName("bIsSupportMode"));
}
