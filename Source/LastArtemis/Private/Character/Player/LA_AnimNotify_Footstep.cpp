#include "Character/Player/LA_AnimNotify_Footstep.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

ULA_AnimNotify_Footstep::ULA_AnimNotify_Footstep(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
#if WITH_EDITOR
    bShouldFireInEditor = false;
#endif
}

void ULA_AnimNotify_Footstep::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (MeshComp == nullptr || MeshComp->GetWorld() == nullptr) return;

    AActor* WeaponActor = MeshComp->GetOwner();
    if (WeaponActor == nullptr) return;

    ACharacter* OwnerCharacter = Cast<ACharacter>(WeaponActor->GetOwner());
    if (OwnerCharacter == nullptr && MeshComp->GetAttachParent())
    {
        OwnerCharacter = Cast<ACharacter>(MeshComp->GetAttachParent()->GetOwner());
    }

    if (OwnerCharacter == nullptr) return;

    // 플레이어가 공중에 떠 있는 상태라면 발소리를 생략
    UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement();
    if (Movement && Movement->IsFalling()) return;

    // 플레이어가 움직이지 않으면 발소리를 생략
    if (OwnerCharacter->GetVelocity().Size2D() < 10.f) return;

    FVector StartLocation = OwnerCharacter->GetActorLocation();
    FVector EndLocation = StartLocation - FVector(0.f, 0.f, TraceDistance);

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(WeaponActor);
    QueryParams.AddIgnoredActor(OwnerCharacter);
    QueryParams.bReturnPhysicalMaterial = true;

    FHitResult HitResult;
    bool bHit = MeshComp->GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, QueryParams);

    if (bHit)
    {
        EPhysicalSurface SurfaceType = SurfaceType_Default;

        if (HitResult.PhysMaterial.IsValid())
        {
            SurfaceType = HitResult.PhysMaterial->SurfaceType;
        }

        if (USoundBase** SoundToPlay = Sounds.Find(SurfaceType))
        {
            if (*SoundToPlay)
            {
                UGameplayStatics::PlaySoundAtLocation(MeshComp->GetWorld(), *SoundToPlay, HitResult.ImpactPoint);
            }
        }
    }
}
