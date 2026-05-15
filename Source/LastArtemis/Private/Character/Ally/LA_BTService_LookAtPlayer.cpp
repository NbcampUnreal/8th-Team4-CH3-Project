// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Ally/LA_BTService_LookAtPlayer.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"


ULA_BTService_LookAtPlayer::ULA_BTService_LookAtPlayer()
{
    NodeName = TEXT("Look At Player");
    InterpSpeed = 5.f;
}

void ULA_BTService_LookAtPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController) return;

    APawn* OwnerPawn = AIController->GetPawn();
    if (!OwnerPawn) return;

    AActor* Player = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(FName("PlayerActor")));
    if (!Player) return;

    if (OwnerPawn->GetVelocity().IsNearlyZero())
    {
        FRotator CurrentRotation = AIController->GetControlRotation();
        FRotator PlayerRotation = Player->GetActorRotation();
        FRotator NewRotation = FMath::RInterpTo(CurrentRotation, PlayerRotation, DeltaSeconds, InterpSpeed);
        AIController->SetControlRotation(NewRotation);
    }
}
