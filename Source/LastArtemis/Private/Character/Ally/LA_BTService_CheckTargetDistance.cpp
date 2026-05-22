// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Ally/LA_BTService_CheckTargetDistance.h"


#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Character/Ally/LA_AllyAIController.h"
#include "Character/Enemy/LA_EnemyCharacter.h"
#include "Kismet/GameplayStatics.h"

ULA_BTService_CheckTargetDistance::ULA_BTService_CheckTargetDistance()
{
    NodeName = TEXT("Check Target Distance (Aggro)");
    Interval = 0.5f;
    RandomDeviation = 0.1f;
    LoseSightDistance = 5000.f;

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


    //UE_LOG(LogTemp, Display, TEXT("CheckTargetDistance Service Ticking"));
    // 현재 타겟 가져오기
    AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(FName("TargetActor")));
    bool bIsCommandedTarget = Blackboard->GetValueAsBool(FName("IsCommandedTarget"));

    // 기존 타겟 거리 검사 및 놓침 처리용 변수
    float CurrentTargetDistanceSq = 99999999.f;
    float LoseSightDistSq = FMath::Square(LoseSightDistance);

    // 기존 타겟이 존재할 때 처리
    if (Target)
    {
        CurrentTargetDistanceSq = FVector::DistSquared(OwnerPawn->GetActorLocation(), Target->GetActorLocation());

        ALA_BaseCharacter* TargetBase = Cast<ALA_BaseCharacter>(Target);
        bool bIsTargetDead = TargetBase ? TargetBase->bIsDead : false;

        if (CurrentTargetDistanceSq > LoseSightDistSq || bIsTargetDead)
        {
            Blackboard->ClearValue(FName("TargetActor"));
            Blackboard->SetValueAsBool(FName("IsCommandedTarget"), false);
            AIController->ClearFocus(EAIFocusPriority::Gameplay);
            AIController->ClearFocus(EAIFocusPriority::Move);
            AIController->ClearFocus(EAIFocusPriority::Default);
            Target = nullptr;
            CurrentTargetDistanceSq = 99999999.f;
            UE_LOG(LogTemp, Warning, TEXT("적 타겟을 놓쳤습니다! 거리가 너무 멉니다."));
        }

    }

    // 명령받은 타겟이 살아있으면 탐색하지 않고 스킵
    if (bIsCommandedTarget && Target != nullptr)
    {
        return;
    }

    // 타겟이 존재하지 않을 때 새로운 타겟 찾기
    TArray<AActor*> AllEnemies;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALA_BaseCharacter::StaticClass(), AllEnemies);

    AActor* BestTarget = Target;
    float MinDistSq = CurrentTargetDistanceSq;
    // 타겟 전환에 필요한 최소 거리 차이
    float SwitchThresholdSq = FMath::Square(150.f);

    for (AActor* Actor : AllEnemies)
    {
        if (Actor == OwnerPawn) continue;

        ALA_BaseCharacter* Enemy = Cast<ALA_BaseCharacter>(Actor);

        if (!Enemy || Enemy->bIsDead) continue;
        if (Enemy->CharacterTags.HasTag(FGameplayTag::RequestGameplayTag(FName("Team.Ally")))) continue;

        float DistToEnemySq = FVector::DistSquared(OwnerPawn->GetActorLocation(), Enemy->GetActorLocation());

        if (DistToEnemySq < LoseSightDistSq)
        {

            if (Target == nullptr || DistToEnemySq < (MinDistSq - SwitchThresholdSq))
            {
                MinDistSq = DistToEnemySq;
                BestTarget = Enemy;
            }
        }
    }


    if (BestTarget != nullptr && BestTarget != Target)
    {
        Blackboard->SetValueAsObject(FName("TargetActor"), BestTarget);
    }
}


