// Fill out your copyright notice in the Description page of Project Settings.


#include "Trap/LA_ShieldDrainingArea.h"
#include "Character/LA_BaseCharacter.h"


// Sets default values
ALA_ShieldDrainingArea::ALA_ShieldDrainingArea()
{
    // 기본 감소량 설정
    ShieldDrainAmount = 5.0f;
    TickInterval = 0.2f; // 쉴드는 오염도보다 조금 더 빠르게 깎이도록 설정 (연사 느낌)
}

void ALA_ShieldDrainingArea::ApplyEffect(AActor* TargetActor)
{
    ALA_BaseCharacter* Player = Cast<ALA_BaseCharacter>(TargetActor);

    if (Player)
    {
        // 쉴드가 남아있을 때만 감소 로직 실행
        if (Player->GetCurrentShield() > 0.0f)
        {
            // 아까 플레이어 베이스에 만들어둔 쉴드 전용 감소 함수 호출
            Player->ReduceShieldOnly(ShieldDrainAmount);

            UE_LOG(LogTemp, Warning, TEXT("Shield is Draining! Remaining: %f"), Player->GetCurrentShield());
        }
        else
        {
            // 쉴드가 다 깎였을 때 로그 (필요 시 여기서 체력을 깎게 변경도 가능)
            UE_LOG(LogTemp, Log, TEXT("Player Shield is already 0."));
        }
    }
}

