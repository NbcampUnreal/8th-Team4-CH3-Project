// Fill out your copyright notice in the Description page of Project Settings.


#include "LA_PlayerWeaponHolderComponent.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "LastArtemis/Weapon/LA_WeaponBase.h"
#include "LA_Holder.h"

// Sets default values for this component's properties
ULA_PlayerWeaponHolderComponent::ULA_PlayerWeaponHolderComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
    AimCamera = CreateDefaultSubobject<UCameraComponent>(FName("AimCamera"));

    // 해당 컴포넌트를 가진 액터가 존재하는지 확인
    if (AActor* OwnerActor = GetOwner())
    {
        Character = Cast<ACharacter>(OwnerActor);
    }
}


// Called when the game starts
void ULA_PlayerWeaponHolderComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void ULA_PlayerWeaponHolderComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void ULA_PlayerWeaponHolderComponent::ActivateWeapon(AActor* WeaponActor, USkeletalMesh* WeaponEquipedCharacterMesh, UAnimInstance* WeaponAnimInstance)
{
    // 장착하려는 무기와 중복되는 인덱스 찾기
    int32 DuplicatedIndex = WeaponsContainer.IndexOfByPredicate([&](const AActor* Element)
        {
            return Element == WeaponActor;
        });

    // 기존에 보유한 무기인 경우
    if (DuplicatedIndex != INDEX_NONE)
    {
        // 현재 장착중인 무기와 겹치는지 확인
        if (EquipedIndex == DuplicatedIndex)
        {
            // 함수 조기 종료
            return;
        }

        // 해당 무기 인덱스 저장
        EquipedIndex = DuplicatedIndex;
    }
    else
    {
        // 해당 무기 인덱스 저장
        EquipedIndex = WeaponsContainer.Num();

        // 획득한 무기 저장
        WeaponsContainer.Add(WeaponActor);
    }

    // 무기 할당 전 기존 무기 해제
    DeactivateWeapon();

    if (USkeletalMeshComponent* CharacterMeshComponent = Character->GetMesh())
    {
        // 캐릭터의 스켈레탈 메쉬 설정
        if (WeaponEquipedCharacterMesh != nullptr)
        {
            CharacterMeshComponent->SetSkeletalMesh(WeaponEquipedCharacterMesh);
        }

        // 캐릭터의 애니메이션 인스턴스 설정
        if (WeaponAnimInstance != nullptr)
        {
            CharacterMeshComponent->AnimScriptInstance = WeaponAnimInstance;
        }
    }

    // 조준경 위치에 카메라 부착
    AttachAimCamera();

    return;
}

void ULA_PlayerWeaponHolderComponent::DeactivateWeapon()
{
    if (USkeletalMeshComponent* CharacterMeshComponent = Character->GetMesh())
    {
        // 캐릭터의 애니메이션 인스턴스 제거
        CharacterMeshComponent->ClearAnimScriptInstance();
        // 캐릭터의 스켈레탈 메쉬 제거
        CharacterMeshComponent->SetSkeletalMesh(nullptr);
    }

    return;
}

void ULA_PlayerWeaponHolderComponent::StartFire()
{
    // 장착한 무기가 유효한지 확인
    if (WeaponsContainer.IsValidIndex(EquipedIndex) == false)
    {
        return;
    }

    // 장착한 무기 발사 시작
    if (ALA_WeaponBase* EquipedWeapon = Cast<ALA_WeaponBase>(WeaponsContainer[EquipedIndex]))
    {
        EquipedWeapon->StartFire();
    }

    //// 발사 키 입력 상태 저장
    //bIsFired = true;

    //// 총알 발사할 수 있는지 확인
    //if (bCanFire == true)
    //{
    //    // 즉시 무기 발사
    //    Fire();
    //    return;
    //}

    return;
}

void ULA_PlayerWeaponHolderComponent::StopFire()
{
    // 장착한 무기가 유효한지 확인
    if (WeaponsContainer.IsValidIndex(EquipedIndex) == false)
    {
        return;
    }

    // 장착한 무기 발사 중단
    if (ALA_WeaponBase* EquipedWeapon = Cast<ALA_WeaponBase>(WeaponsContainer[EquipedIndex]))
    {
        EquipedWeapon->StopFire();
    }

    //bIsFired = false;
    //return;
}

void ULA_PlayerWeaponHolderComponent::Fire()
{
    // 발사 키를 입력 중인지 확인
    // 총알을 발사할 수 있는지 확인
    if (bIsFired == false || bCanFire == false)
    {
        return;
    }

    // 총알을 발사할 수 없도록 설정
    bCanFire = false;

    // 총구 위치 얻기
    FVector startLocation = Character->GetMesh()->GetSocketLocation(FName("Muzzle"));
    // 목표 지점 얻기
    //FVector endLocation = ILA_Holder::Execute_GetFocusLocation(Character);
    FVector endLocation = FVector::ZeroVector;

    // 총알 궤적 그리기
    DrawDebugLine(GetWorld(), startLocation, endLocation, FColor::Red, false, 3.f, 0, 1);

    // 반동 적용
    Character->AddControllerPitchInput(RecoilAmount);

    // 애니메이션 몽타주 재생
    Character->PlayAnimMontage(FireMontage);
}

void ULA_PlayerWeaponHolderComponent::Reload()
{
    // 총알을 발사할 수 없도록 설정
    bCanFire = false;

    // 애니메이션 몽타주 재생
    Character->PlayAnimMontage(ReloadMontage);
}

void ULA_PlayerWeaponHolderComponent::NotifyCanFire()
{
    // 총알을 발사할 수 있도록 변경
    bCanFire = true;

    // 발사 키 입력중인지 확인
    // 자동으로 발사되는 무기인지 확인
    if (bIsFired == true && bIsAuto == true)
    {
        // 다시 총알 발사
        Fire();
    }
}

void ULA_PlayerWeaponHolderComponent::AttachAimCamera()
{
    if (AimCamera == nullptr)
    {
        AimCamera = CreateDefaultSubobject<UCameraComponent>(FName("AimCamera"));
    }

    // 조준경 위치에 카메라 부착
    if (Character != nullptr)
    {
        if (USkeletalMeshComponent* CharacterMeshComponent = Character->GetMesh())
        {
            AimCamera->AttachToComponent(CharacterMeshComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("AimEyeSocket"));
        }
    }
}

