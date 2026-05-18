// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Ally/LA_BTService_CheckTargetDistance.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Character/Ally/LA_AllyAIController.h"

ULA_BTService_CheckTargetDistance::ULA_BTService_CheckTargetDistance()
{
    NodeName = TEXT("Check Target Distance (Aggro)");
    Interval = 0.5f;
    RandomDeviation = 0.1f;
    LoseSightDistance = 1500.f;

    bCreateNodeInstance = true;
}

void ULA_BTService_CheckTargetDistance::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
    float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    ALA_AllyAIController* AIController = Cast<ALA_AllyAIController>(OwnerComp.GetAIOwner());
    APawn* OwnerPawn = AIController ? AIController->GetPawn() : nullptr;
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();

    if (!AIController || !OwnerPawn || !Blackboard) return;

    UE_LOG(LogTemp, Display, TEXT("CheckTargetDistance Service Ticking"));
    // 현재 타겟 가져오기
    AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(FName("TargetActor")));
    if (Target)
    {
        bool bIsCommandedTarget = Blackboard->GetValueAsBool(FName("IsCommandedTarget"));
        UE_LOG(LogTemp, Warning, TEXT("IsCommandedTarget: %s"), bIsCommandedTarget ? TEXT("true") : TEXT("false"));

        if (bIsCommandedTarget)
        {
            return;
        }

        float Distance = FVector::Dist(OwnerPawn->GetActorLocation(), Target->GetActorLocation());

        if (Distance > LoseSightDistance)
        {
            Blackboard->ClearValue(FName("TargetActor"));

            AIController->ClearFocus(EAIFocusPriority::Gameplay);
            AIController->ClearFocus(EAIFocusPriority::Move);
            AIController->ClearFocus(EAIFocusPriority::Default);

            UE_LOG(LogTemp, Warning, TEXT("Target Lost! Too far away"));
        }
    }

}


