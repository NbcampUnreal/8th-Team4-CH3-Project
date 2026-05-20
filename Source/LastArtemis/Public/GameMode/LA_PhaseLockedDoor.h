// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Object/LA_Interactable.h"
#include "LA_PhaseLockedDoor.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class USphereComponent;
class UWidgetComponent;
class APawn;

UCLASS(BlueprintType)
class LASTARTEMIS_API ALA_PhaseLockedDoor : public AActor, public ILA_Interactable
{
	GENERATED_BODY()
	
public:	
	ALA_PhaseLockedDoor();

protected:
	virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

protected:
    // 플레이어 상호 작용 범위에 들어왔을 때
    UFUNCTION()
    void OnInteractionBeginOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );

    // 플레이어 상호 작용 범위를 나갈 때
    UFUNCTION()
    void OnInteractionEndOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex
    );

    // F 입력으로 상호 작용 시 호출
    virtual void Interact_Implementation(AActor* InteractInstigator) override;

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
    // F 상호작용 안내 위젯
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UWidgetComponent* InteractableIndicator;

protected:
    ////////////////////
    // 오버랩
    ////////////////////
    // 플레이어가 상호 작용 가능한 범위 안에 있는지
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Overlap")
    bool bPlayerInRange;
    // 현재 범위 안에 있는 플레이어
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Overlap")
    APawn* OverlappingPlayerPawn;

protected:
    ////////////////////
    // Door 로직
    ////////////////////
    
    // 문이 완전히 열렸는지
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
    bool bOpened;
    // 문이 열리는 중인지
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
    bool bOpening;
    // 현재 Phase Index가 완료되어야 문 열림
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door")
    int32 RequiredClearedPhaseIndex;
    // 문이 열릴 때 이동 목표 위치
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door")
    FVector OpenLocationOffset;
    // 보간 속도
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door")
    float OpenInterpSpeed;

protected:
    // 잠김 안내 표시 시간
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door|UI")
    float LockedMessageDisplayTime;

private:
    // 현재 GameState 기준으로 문을 열 수 있는지 검사
    bool CanOpenDoor() const;
    // 문 열기
    void OpenDoor();
    // 게임 로드 시 문 열린 상태로 복원
    void ApplyInitialDoorState();

private:
    ////////////////////
    // 잠김 메시지 표시
    ////////////////////

    // LockedMessageText 표시
    void ShowLockedMessage();
    // LockedMessageText 숨김
    void HideLockedMessage();

    // 잠김 안내 문구 자동 숨김용 타이머
    FTimerHandle LockedMessageTimerHandle;

private:
    // 닫힌 상태의 문 상대 위치
    FVector ClosedDoorLocation;
    // 열린 상태의 문 상대 위치
    FVector OpenedDoorLocation;
};
