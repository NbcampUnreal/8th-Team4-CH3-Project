// Fill out your copyright notice in the Description page of Project Settings.


#include "Trap/LA_ShieldDrainingArea.h"
#include "Character/LA_BaseCharacter.h"
#include "Character/Player/Component/LA_HealthComponent.h"

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
        // 🎯 1. [심볼 에러 완벽 해결]
        // 플레이어 본체의 제거된 함수 대신, 물려받은 헬스 컴포넌트 파이프라인을 조준합니다!
        ULA_HealthComponent* HealthComp = Player->GetHealthComponent();

        if (HealthComp)
        {
            // 🎯 2. 컴포넌트 내부의 실시간 실드 잔량을 정확히 체크합니다.
            if (HealthComp->GetCurrentShield() > 0.0f)
            {
                // 베이스 캐릭터에 연동해 둔 실드 차감 함수 가동
                Player->ReduceShieldOnly(ShieldDrainAmount);

                UE_LOG(LogTemp, Warning, TEXT("⚡ [트랩 작동] 플레이어 실드 흡수 중! 남은 실드: %f"), HealthComp->GetCurrentShield());
            }
            else
            {
                // 쉴드가 다 깎였을 때 로그
                UE_LOG(LogTemp, Log, TEXT("🛡️ 플레이어 실드가 이미 0입니다. 대미지 변환 대기 상태."));
            }
        }
    }
}
