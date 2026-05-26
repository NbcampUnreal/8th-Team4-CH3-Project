// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Ally/LA_BTTask_FindPatrolLocation.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "AIController.h"
#include "Character/Ally/LA_AllyAI.h"
#include "Character/Ally/LA_AllyAIController.h"
#include "Character/Player/LA_PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"


ULA_BTTask_FindPatrolLocation::ULA_BTTask_FindPatrolLocation()
{
    NodeName = TEXT("Find Random Follow Location");

    BehindDistance = 400.f;
    WanderRadius = 500.f;
    // MinDistance = 300.f;
    SearchRadius = 1500.f;
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
    // if (!Player) return EBTNodeResult::Failed;
    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSystem) return EBTNodeResult::Failed;


    // NavMesh 위 유효한 위치 찾기

    FNavLocation NavLocation;
    FVector FinalDestination;
    FVector ExistingTargetLocation = Blackboard->GetValueAsVector(FName("TargetLocation"));

    // 라인 트레이싱 세팅 (플레이어와 AI 자신은 충돌에서 무시)
    FCollisionQueryParams CollisionParams;
    CollisionParams.AddIgnoredActor(OwnerPawn);
    if (Player) CollisionParams.AddIgnoredActor(Player);

    // 플레이어가 존재할 때: 플레이어 주변에서 순찰

    if (Player)
    {
        // 플레이어와 너무 먼 거리일 시 -> 플레이어 곁으로 텔레포트
        float DistToPlayer = FVector::Dist(OwnerPawn->GetActorLocation(), Player->GetActorLocation());

        if (DistToPlayer > 2000.f)
        {
            FVector TeleportLocation = Player->GetActorLocation() - (Player->GetActorForwardVector() * BehindDistance);

            if (NavSystem->ProjectPointToNavigation(TeleportLocation, NavLocation, FVector(100.f, 100.f, 500.f)))
            {
                FVector SafeSpawnLocation = NavLocation.Location;
                SafeSpawnLocation.Z += 100.f;

                OwnerPawn->SetActorLocation(SafeSpawnLocation);
                Blackboard->SetValueAsVector(FName("TargetLocation"), NavLocation.Location);

                UE_LOG(LogTemp, Warning, TEXT("플레이어와 아군의 거리가 너무 멉니다! 플레이어 곁으로 순간이동 합니다."));
                return EBTNodeResult::Succeeded;

            }
        }

        // 플레이어의 컨트롤러(시선) 방향 기준
        FVector LookDirection = Player->GetActorForwardVector();
        FVector RightDirection = Player->GetActorRightVector();

        if (ALA_PlayerCharacter* PlayerCharacter = Cast<ALA_PlayerCharacter>(Player))
        {
            FRotator ControlRotation = PlayerCharacter->GetControlRotation();
            ControlRotation.Pitch = 0.f;
            ControlRotation.Roll = 0.f;

            LookDirection = FRotationMatrix(ControlRotation).GetUnitAxis(EAxis::X);
            RightDirection = FRotationMatrix(ControlRotation).GetUnitAxis(EAxis::Y);
        }



        // 아군 번호 매기기
        TArray<AActor*> AllAllies;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALA_AllyAI::StaticClass(), AllAllies);

        int32 MySquadIndex = 0;
        if (AllAllies.Contains(OwnerPawn))
        {
            MySquadIndex = AllAllies.Find(OwnerPawn);
        }



        // 진형 (좌우 날개) 위치 계산
        FVector PlayerRight = Player->GetActorRightVector();
        FVector PlayerBackOffset = Player->GetActorLocation() - (LookDirection * BehindDistance);


        // 짝수: 오른쪽 , 홀수: 왼쪽 위치 할당
        bool bIsRightWing = (MySquadIndex % 2 == 0);
        SpreadDistance = 250.f;
        FVector TacticalOffset = bIsRightWing ? (RightDirection * SpreadDistance) : (-RightDirection * SpreadDistance);
        FinalDestination = PlayerBackOffset + TacticalOffset;



        // NavMesh 검사 범위
        FVector Extent = FVector(20.f, 20.f, 50.f);

        // 최종 목적지 담아둘 변수 (디버그용)
        FVector UltimateDestination = Player->GetActorLocation();
        FColor DebugColor = FColor::Red;


        if (NavSystem->ProjectPointToNavigation(FinalDestination, NavLocation, Extent))
        {
            FVector TraceStart = Player->GetActorLocation();
            FVector TraceEnd = NavLocation.Location + FVector(0.f, 0.f, 90.f);
            FHitResult HitResult;
            bool bHitWall = GetWorld()->LineTraceSingleByChannel(
                HitResult,
                TraceStart,
                TraceEnd,
                ECC_WorldStatic,
                CollisionParams
            );

            if (!bHitWall)
            {
                if (FVector::DistSquared(ExistingTargetLocation, NavLocation.Location) < FMath::Square(100.0f))
                {
                    return EBTNodeResult::Succeeded;
                }
                UltimateDestination = NavLocation.Location;
                DebugColor = FColor::Green;

                Blackboard->SetValueAsVector(FName("TargetLocation"), UltimateDestination);
                DrawDebugSphere(GetWorld(), UltimateDestination, 50.f, 12, DebugColor, 1.f);
                return EBTNodeResult::Succeeded;
            }
        }

        // 위 로직 실패시(벽에 막혔을 때) 플레이어 등 뒤로 집합
        if (NavSystem->ProjectPointToNavigation(PlayerBackOffset, NavLocation, FVector(50.f, 50.f, 250.f)))
        {
            FVector TraceStart = Player->GetActorLocation();
            FVector TraceEnd = NavLocation.Location + FVector(0.f, 0.f, 90.f);
            FHitResult HitResult;

            bool bHitWall = GetWorld()->LineTraceSingleByChannel(
                HitResult,
                TraceStart,
                TraceEnd,
                ECC_WorldStatic,
                CollisionParams
                );

            if (!bHitWall)
            {
                UltimateDestination = NavLocation.Location;
                DebugColor = FColor::Orange;

                Blackboard->SetValueAsVector(FName("TargetLocation"), NavLocation.Location);
                DrawDebugSphere(GetWorld(), UltimateDestination, 50.f, 12, DebugColor, false, 0.2f);

                return EBTNodeResult::Succeeded;
            }


        }

        // 최악의 경우 플레이어 위치로 이동
        Blackboard->SetValueAsVector(FName("TargetLocation"), Player->GetActorLocation());
        DrawDebugSphere(GetWorld(), UltimateDestination, 50.f, 12, DebugColor, false, 0.2f);

        return EBTNodeResult::Succeeded;


    }
    else // 플레이어가 존재하지 않을 때 제자리에서 순찰
    {

        if (NavSystem->GetRandomPointInNavigableRadius(OwnerPawn->GetActorLocation(), WanderRadius, NavLocation))
        {
            Blackboard->SetValueAsVector(FName("TargetLocation"), NavLocation.Location);

            DrawDebugSphere(GetWorld(),NavLocation.Location, 50.f, 12, FColor::Cyan, false, 0.2f);
            UE_LOG(LogTemp, Warning, TEXT("시뮬레이션 모드: Random Patrol Location Found!"));

            return EBTNodeResult::Succeeded;
        }

        return EBTNodeResult::Failed;
    }


}
