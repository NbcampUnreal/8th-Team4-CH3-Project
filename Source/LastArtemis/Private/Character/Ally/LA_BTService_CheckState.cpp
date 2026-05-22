// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Ally/LA_BTService_CheckState.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Character/LA_BaseCharacter.h"
#include "Character/Player/LA_PlayerCharacter.h"
#include "Character/Player/Component/LA_HealthComponent.h"
#include "Kismet/GameplayStatics.h"



ULA_BTService_CheckState::ULA_BTService_CheckState()
{
    NodeName = TEXT("Check Player State");
    Interval = 0.5f;
    RandomDeviation = 0.1f;
    AbandonDistance = 1000.f;
}

void ULA_BTService_CheckState::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);


    APawn* OwnerPawn = OwnerComp.GetAIOwner()->GetPawn();
    if (!OwnerPawn) return;

    bool bIsSupportMode = false;

    // 플레이어 캐릭터 체력 상태 체크
    ALA_PlayerCharacter* Player = Cast<ALA_PlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (!Player) return;

    ULA_HealthComponent* HealthComponent = Player->FindComponentByClass<ULA_HealthComponent>();
    if (!HealthComponent) return;

    if (!HealthComponent->IsDead())
    {
        // 체력 50 이하 -> 서포트 모드 true
        bIsSupportMode = HealthComponent->GetHealthPercent() <= 0.5;
        if (bIsSupportMode)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Support Mode Activated"));
        }
    }




    // 블랙보드 값 업데이트
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (!Blackboard) return;

    Blackboard->SetValueAsBool(FName("bIsSupportMode"), bIsSupportMode);

    ALA_BaseCharacter* Target = Cast<ALA_BaseCharacter>(Blackboard->GetValueAsObject(FName("TargetActor")));
    if (!Target) return;

    float DistToPlayer = FVector::Dist(OwnerPawn->GetActorLocation(), Player->GetActorLocation());

    // 플레이어가 너무 멀면 TargetActor 클리어
    if (DistToPlayer > AbandonDistance)
    {
        Blackboard->ClearValue(FName("TargetActor"));

        return;
    }

    if (Target->bIsDead)
    {
        Blackboard->ClearValue(FName("TargetActor"));
    }

}

