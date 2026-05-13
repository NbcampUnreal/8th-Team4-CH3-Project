// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LA_Interactable.h"
#include "LA_RecoveryShower.generated.h"

UCLASS()
class LASTARTEMIS_API ALA_RecoveryShower : public AActor, public ILA_Interactable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALA_RecoveryShower();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
#pragma region Components

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "0_Components", meta = (AllowPrivateAccess = true))
    TObjectPtr<USceneComponent> DefaultSceneRoot;

    // 객체 외형
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "0_Components", meta = (AllowPrivateAccess = true))
    TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

    // 상호작용 안내 UI가 표시되는 영역을 정의한은 Collision
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "0_Components", meta = (AllowPrivateAccess = true))
    TObjectPtr<class USphereComponent> Collision;

    // 상호작용 안내 UI
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "0_Components", meta = (AllowPrivateAccess = true))
    TObjectPtr<class UWidgetComponent> InteractableIndicator;

#pragma endregion

    // 상호작용 가능한 최대 거리
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1_Interact")
    float InteractableDistance;

    // 회복량
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1_Interact")
    float RecoveryAmount;

    // 상호작용이 가능한 거리 안에 위치한 액터 목록
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "1_Interact")
    TSet<AActor*> AdjacentActors;

public:
    // ILA_Interactable을(를) 통해 상속됨
    virtual void Interact_Implementation(AActor* InteractInstigator) override;

protected:
    UFUNCTION()
    void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
