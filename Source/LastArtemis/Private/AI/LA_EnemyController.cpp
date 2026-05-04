#include "AI/LA_EnemyController.h"

#include "GameplayTagAssetInterface.h"
#include "GameplayTagContainer.h"
#include "Perception/AIPerceptionTypes.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionComponent.h"
#include "LA_BaseCharacter.h"

ALA_EnemyController::ALA_EnemyController()
{
	PrimaryActorTick.bCanEverTick = true;

    // 인식 범위 관련 변수 초기화
	SightRadius = 1500.0f;
	LoseSightRadius = 1800.0f;
	PeripheralVisionAngleDegrees = 90.0f;

	EnemyPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
	SetPerceptionComponent(*EnemyPerceptionComponent);

    // 시야(Sight) 감지 설정 생성 및 세팅
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	if (SightConfig)
	{
	    // 시야 반경 및 시야각 적용
		SightConfig->SightRadius = SightRadius;
		SightConfig->LoseSightRadius = LoseSightRadius;
		SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngleDegrees;

	    // 소속 판별 필터 (적군, 중립, 아군 모두 감지)
		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

	    // Perception 컴포넌트에 감지 센서 적용 및 기본 센서로 지정
		EnemyPerceptionComponent->ConfigureSense(*SightConfig);
		EnemyPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
	}

    // 기본 전투 속성 초기화
	AttackRange = 200.0f;
	AttackDelay = 1.5f;
	TargetPlayer = nullptr;
	bIsAttacking = false;
}

void ALA_EnemyController::BeginPlay()
{
	Super::BeginPlay();

	if (EnemyPerceptionComponent)
	{
		EnemyPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ALA_EnemyController::OnPerceptionUpdated);
	}

}

void ALA_EnemyController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void ALA_EnemyController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (TargetPlayer && GetPawn())
	{
		float Distance = GetPawn()->GetDistanceTo(TargetPlayer);

		if (Distance <= AttackRange)
		{
			AttackPlayer();
		}
		else
		{
			ChasePlayer();
		}
	}
}

void ALA_EnemyController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (Stimulus.WasSuccessfullySensed())
    {
        // 대상의 태그를 조회하기 위해 인터페이스 캐스팅
        IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(Actor);
        if (TagInterface)
        {
            FGameplayTagContainer OwnedTags;
            TagInterface->GetOwnedGameplayTags(OwnedTags);

            // Team.Ally 태그를 가진 대상인지 확인
            FGameplayTag AllyTag = FGameplayTag::RequestGameplayTag(FName("Team.Ally"));
            if (OwnedTags.HasTag(AllyTag))
            {
                TargetPlayer = Actor; // 아군(Ally)을 타겟으로 설정
                return;
            }
        }

        // 기존 플레이어 타겟팅 로직 (필요시)
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
        if (Actor == PlayerPawn)
        {
            TargetPlayer = Actor;
        }
    }
    else
    {
        TargetPlayer = nullptr;
        StopMovement();
    }
}


void ALA_EnemyController::ChasePlayer()
{
	if (TargetPlayer)
	{
		MoveToActor(TargetPlayer, AttackRange - 20.0f);
	}
}

void ALA_EnemyController::AttackPlayer()
{
	if (bIsAttacking) return;

	StopMovement();
	bIsAttacking = true;

	// 타겟 플레이어에게 데미지 부여 로직
	ALA_BaseCharacter* TargetCharacter = Cast<ALA_BaseCharacter>(TargetPlayer);
	if (TargetCharacter)
	{
		// 쉴드 및 체력 차감 로직 수행 (기본 공격력 등은 캐릭터나 컨트롤러에서 지정)
		TargetCharacter->TakeDamageCustom(10.0f);
	}

	// 공격 몽타주 재생 함수 등을 여기에 추가 가능합니다.

	// 쿨다운 타이머 설정
	GetWorld()->GetTimerManager().SetTimer(
		AttackTimerHandle,
		this,
		&ALA_EnemyController::ResetAttackState,
		AttackDelay,
		false
	);
}

void ALA_EnemyController::ResetAttackState()
{
	bIsAttacking = false;
}

