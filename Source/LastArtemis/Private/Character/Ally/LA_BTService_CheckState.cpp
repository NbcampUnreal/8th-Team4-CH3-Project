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
}

void ULA_BTService_CheckState::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);


    if (APawn* ControllingPawn = OwnerComp.GetAIOwner()->GetPawn())
    {



        bool bIsSupportMode = false;

        // 플레이어 캐릭터 체력 상태 체크
        if (ALA_PlayerCharacter* Player = Cast<ALA_PlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)))
        {
            if (ULA_HealthComponent* HealthComponent = Player->FindComponentByClass<ULA_HealthComponent>())
            {

                UE_LOG(LogTemp, Warning, TEXT("HP: %f / %f (Percent: %f)"),
                HealthComponent->GetCurrentHealth(),
                HealthComponent->GetMaxHealth(),
                HealthComponent->GetHealthPercent());

                if (!HealthComponent->IsDead())
                {
                    // 체력 50 이하 -> 서포트 모드 true
                    bIsSupportMode = HealthComponent->GetHealthPercent() <= 0.5;
                    if (bIsSupportMode)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Support Mode Activated"));
                    }


                }

            }



        }





        // 블랙보드 값 업데이트
        if (UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent())
        {
            if (ALA_BaseCharacter* Target = Cast<ALA_BaseCharacter>(
                Blackboard->GetValueAsObject(FName("TargetActor"))
            ))
            {
                if (Target->bIsDead)
                {
                    Blackboard->ClearValue(FName("TargetActor"));
                }
            }
            Blackboard->SetValueAsBool(FName("bIsSupportMode"), bIsSupportMode);
        }



    }




}

