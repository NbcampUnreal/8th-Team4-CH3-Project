// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Ally/LA_BTTask_LookAround.h"

#include "Character/Ally/LA_AllyAIController.h"

ULA_BTTask_LookAround::ULA_BTTask_LookAround()
{
    NodeName = TEXT("Look Around");
    bNotifyTick = true;
}

EBTNodeResult::Type ULA_BTTask_LookAround::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{

    return EBTNodeResult::InProgress;
}

void ULA_BTTask_LookAround::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    if (AAIController* AIController = OwnerComp.GetAIOwner())
    {
        if (APawn* OwnerPawn = AIController->GetPawn())
        {
            // 현재 회전에 RotationSpeed 더하기
            FRotator CurrentRotation = OwnerPawn->GetActorRotation();
            CurrentRotation.Yaw += RotationSpeed * DeltaSeconds;
            OwnerPawn->SetActorRotation(CurrentRotation);
        }
    }

    // Look Duration 후 완료
    // NodeMemory로 시간 추적 필요
    FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
}
