#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Chaos/ChaosEngineInterface.h"
#include "LA_AnimNotify_Footstep.generated.h"

UCLASS()
class LASTARTEMIS_API ULA_AnimNotify_Footstep : public UAnimNotify
{
	GENERATED_BODY()

public:
    ULA_AnimNotify_Footstep(const FObjectInitializer& ObjectInitializer);

    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
    // 재질별 사운드
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound Settings")
    TMap<TEnumAsByte<EPhysicalSurface>, class USoundBase*> Sounds;

    // 라인트레이스로 바닥을 검사할 거리
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound Settings")
    float TraceDistance = 200.f;
};
