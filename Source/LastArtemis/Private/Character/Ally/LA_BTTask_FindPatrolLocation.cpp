// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Ally/LA_BTTask_FindPatrolLocation.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "AIController.h"
#include "SNegativeActionButton.h"
#include "Character/Ally/LA_AllyAI.h"
#include "Character/Ally/LA_AllyAIController.h"
#include "Kismet/GameplayStatics.h"

ULA_BTTask_FindPatrolLocation::ULA_BTTask_FindPatrolLocation()
{
    NodeName = TEXT("Find Random Follow Location");

    BehindDistance = 400.f;
    WanderRadius = 500.f;
    // MinDistance = 300.f;
    // SearchRadius = 800.f;
}

EBTNodeResult::Type ULA_BTTask_FindPatrolLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    ALA_AllyAIController* AIController = Cast<ALA_AllyAIController>(OwnerComp.GetAIOwner());
    if (!AIController) return EBTNodeResult::Failed;
    APawn* OwnerPawn = AIController->GetPawn();
    if (!OwnerPawn) return EBTNodeResult::Failed;
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (!Blackboard) return EBTNodeResult::Failed;
    AActor* Player = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(FName("PlayerActor")));
    if (!Player) return EBTNodeResult::Failed;
    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSystem) return EBTNodeResult::Failed;

    FVector PlayerLocation = Player->GetActorLocation();
    FVector PlayerForward = Player->GetActorForwardVector();
    FVector PlayerRight = Player->GetActorRightVector();

    // 플레이어 뒤쪽 기준점 계산
    FVector PlayerBackOffset = Player->GetActorLocation()
    - (Player->GetActorForwardVector() * BehindDistance);

    // 아군 번호 매기기
    TArray<AActor*> AllAllies;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALA_AllyAI::StaticClass(), AllAllies);

    int32 MySquadIndex = 0;
    if (AllAllies.Contains(OwnerPawn))
    {
        MySquadIndex = AllAllies.Find(OwnerPawn);
    }

    // 짝수: 오른쪽 , 홀수: 왼쪽 위치 할당
    bool bIsRightWing = (MySquadIndex % 2 == 0);

    SpreadDistance = 250.f;
    FVector TacticalOffset = bIsRightWing ? (PlayerRight * SpreadDistance) : (-PlayerRight * SpreadDistance);

    FVector FinalDestination = PlayerBackOffset + TacticalOffset;

    // NavMesh 위 유효한 위치 찾기
    FNavLocation NavLocation;
    NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());

    if (NavSystem && NavSystem->ProjectPointToNavigation(FinalDestination, NavLocation))
    {
        // 목적지 저장
        Blackboard->SetValueAsVector(FName("TargetLocation"), NavLocation.Location);

        // 디버그 : AI 목적지 표시
        DrawDebugSphere(GetWorld(), NavLocation.Location, 50.f, 12, FColor::Green, false, 1.f);
        // 디버그 : 플레이어 등 뒤 기준점(파란색)부터 탐색 반경(빨간색)을 그려서 확인
        DrawDebugCircle(GetWorld(), PlayerBackOffset, WanderRadius, 36, FColor::Red, false, 1.f, 0, 2.f, FVector(0,1,0), FVector(1,0,0), false);

        UE_LOG(LogTemp, Warning, TEXT("Follow Location Found!"));
        return EBTNodeResult::Succeeded;

    }
    return EBTNodeResult::Failed;

}
