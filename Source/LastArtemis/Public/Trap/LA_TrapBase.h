// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LA_TrapBase.generated.h"

UCLASS()
class LASTARTEMIS_API ALA_TrapBase : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ALA_TrapBase();

protected:

    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UBoxComponent* TrapArea;

    UPROPERTY(EditAnywhere, Category = "Settings")
    float TickInterval; // 효과가 적용되는 간격

    UPROPERTY()
    TArray<AActor*> OverlappingActors;

    FTimerHandle TrapTickTimerHandle;

    // 영역에 들어오고 나갈 때 호출될 함수
    UFUNCTION()
    virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // 주기적으로 실행될 함수
    void OnTrapTick();

    // 실제 효과를 담당할 가상 함수 (자식에서 구현)
    virtual void ApplyEffect(AActor* TargetActor) {}
};
