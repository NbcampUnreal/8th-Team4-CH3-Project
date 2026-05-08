// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Ally/LA_BTService_CheckState.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Character/LA_BaseCharacter.h"
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
        
        // 플레이어 캐릭터 찾기 (GetPlayerCharacter)
        // 플레이어 HP 및 오염도 체크

        bool bIsSupportMode = false;

        if (ALA_BaseCharacter* PlayerCharacter = Cast<ALA_BaseCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)))
        {
            
            if (!PlayerCharacter->bIsDead)
            {
                bIsSupportMode = (PlayerCharacter->CurrentHealth / PlayerCharacter->MaxHealth) <= 0.3;
            }
        }


        // 플레이어 체력 체크




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
