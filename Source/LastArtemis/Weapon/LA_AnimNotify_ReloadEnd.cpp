#include "LA_AnimNotify_ReloadEnd.h"
#include "LA_WeaponBase.h"

void ULA_AnimNotify_ReloadEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (MeshComp && MeshComp->GetOwner())
    {
        if (ALA_WeaponBase* Weapon = Cast<ALA_WeaponBase>(MeshComp->GetOwner()))
        {
            Weapon->UpdateAmmo();
        }
    }
}
