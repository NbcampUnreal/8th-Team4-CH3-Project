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

    // 🎯 1. 플레이어 본체에 이미 잘 붙어있는 HealthComponent를 조준합니다.
    ULA_HealthComponent* PlayerHealthComp = Player->FindComponentByClass<ULA_HealthComponent>();
    if (!PlayerHealthComp) return;

    if (!PlayerHealthComp->IsDead())
    {
        // 체력 50% 이하 -> 서포트 모드 true (명시적으로 0.5f 표기)
        bIsSupportMode = PlayerHealthComp->GetHealthPercent() <= 0.5f;
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

    // 플레이어가 너무 멀면 TargetActor 클리어 (플레이어 보호 최우선 기믹)
    if (DistToPlayer > AbandonDistance)
    {
        Blackboard->ClearValue(FName("TargetActor"));
        return;
    }

    // 🎯 2. [심볼 에러 완벽 해결]
    // 적 타겟 본체의 bIsDead 변수를 직접 호출하지 않고, 적이 들고 있는 헬스 컴포넌트의 IsDead() 함수를 조준합니다!
    if (Target->GetHealthComponent() && Target->GetHealthComponent()->IsDead())
    {
        Blackboard->ClearValue(FName("TargetActor"));
    }
}
