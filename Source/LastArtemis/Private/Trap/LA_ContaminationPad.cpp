// Fill out your copyright notice in the Description page of Project Settings.


#include "Trap/LA_ContaminationPad.h"
#include "Character/LA_BaseCharacter.h"
#include "Character/Player/Component/LA_HealthComponent.h"

// Sets default values
ALA_ContaminationPad::ALA_ContaminationPad()
{
    // 기본 증가량 설정
    ContaminationAmount = 1.0f;
    TickInterval = 0.5f; // 0.5초마다 실행
}

void ALA_ContaminationPad::ApplyEffect(AActor* TargetActor)
{
    if (TargetActor == nullptr)
    {
        return;
    }

    // TargetActor가 소유한 HealthComponent 확인
    ULA_HealthComponent* HealthComponent = TargetActor->FindComponentByClass<ULA_HealthComponent>();

    if (HealthComponent != nullptr)
    {
        // 오염도 증가 함수 호출
        HealthComponent->AddContamination(ContaminationAmount);

        // 디버그용 로그
        UE_LOG(LogTemp, Warning, TEXT("Player is standing on Contamination Pad! +%f"), ContaminationAmount);
    }
    //// TargetActor를 플레이어 베이스 클래스로 캐스팅
    //ALA_BaseCharacter* Player = Cast<ALA_BaseCharacter>(TargetActor);

    //if (Player)
    //{
    //    // 아까 플레이어 베이스에 만들어둔 함수 호출
    //    Player->IncreaseContamination(ContaminationAmount);

    //    // 디버그용 로그
    //    UE_LOG(LogTemp, Warning, TEXT("Player is standing on Contamination Pad! +%f"), ContaminationAmount);
    //}
}

