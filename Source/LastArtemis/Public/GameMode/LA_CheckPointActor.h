// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Object/LA_Interactable.h"
#include "Item/LA_ItemDataAsset.h"
#include "LA_CheckPointActor.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class USphereComponent;
class UWidgetComponent;
class APawn;
class ALA_AllyAISpawner;
class USoundBase;

UCLASS(BlueprintType)
class LASTARTEMIS_API ALA_CheckPointActor : public AActor, public ILA_Interactable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALA_CheckPointActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

protected:
    // Check Point 범위 안으로 들어올 때 상호작용 가능하도록
    UFUNCTION()
    void OnInteractionBeginOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );

    UFUNCTION()
    void OnInteractionEndOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex
    );

public:
    // F 입력 시 호출할 함수
    // CheckPoint와 상호작용
    // 세이브, 체력 회복 등등
    UFUNCTION(BlueprintCallable, Category = "Check Point")
    void InteractCheckPoint(APawn* InteractingPawn);

    // 플레이어 체력 회복
    void RecoverPlayer(APawn* InteractingPawn);
    // 플레이어 오염도 제거
    void Decontaminate(APawn* InteractingPawn);
    // 아이템 보급
    void RefillItems(APawn* InteractingPawn);
    // 탄약 보급
    void RefillAmmo(APawn* InteractingPawn);

    void RespawnAllies(APawn* InteractingPawn);

protected:
    ////////////////////
    // 컴포넌트
    ////////////////////
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* SceneComponent;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* StaticMeshComponent;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* InteractionSphere;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UWidgetComponent* InteractableIndicator;

    ////////////////////
    // 오버랩
    ////////////////////
    // 플레이어가 상호 작용 가능한 범위 안에 있는지
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Check Point")
    bool bPlayerInRange;
    // 현재 범위 안에 있는 플레이어
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Check Point")
    APawn* OverlappingPlayerPawn;

    virtual void Interact_Implementation(AActor* InteractInstigator) override;


    // 회복량
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recovery")
    float RecoveryAmount;
    // 오염도 제거
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recovery")
    float DecontaminationAmount;

    ////////////////////
    // 아이템
    ////////////////////
    // 체력 회복 아이템
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    TObjectPtr<ULA_ItemDataAsset> HealingItemData;
    // 오염도 제거 아이템
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    TObjectPtr<ULA_ItemDataAsset> DecontaminationItemData;

    ////////////////////
    // 아군 AI
    ////////////////////
    // 아군 AI
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ally Spawn")
    TObjectPtr<ALA_AllyAISpawner> AllySpawner;

    ////////////////////
    // 사운드
    ////////////////////
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
    TObjectPtr<USoundBase> CheckPointSound;
};
