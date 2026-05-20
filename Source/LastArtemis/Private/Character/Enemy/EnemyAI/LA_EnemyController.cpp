#include "Character/Enemy/EnemyAI/LA_EnemyController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "GameplayTagAssetInterface.h"
#include "Character/Enemy/LA_EnemyCharacter.h"


void ALA_EnemyController::BeginPlay()
{
    Super::BeginPlay();

    if (EnemyPerceptionComponent && SightConfig)
    {
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

        EnemyPerceptionComponent->ConfigureSense(*SightConfig);
    }

    if (GetAIPerceptionComponent())
    {
        GetAIPerceptionComponent()->OnTargetPerceptionUpdated.RemoveDynamic(this, &ALA_EnemyController::OnTargetDetected);
        GetAIPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &ALA_EnemyController::OnTargetDetected);
    }

}

ALA_EnemyController::ALA_EnemyController()
{
    // AIPerception 컴포넌트 초기화
    EnemyPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("EnemyPerceptionComponent"));
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

    if (SightConfig)
    {

        // 헤더에서 설정한 변수값 적용 (블루프린트 수정 가능)
        SightConfig->SightRadius = 1500.0f;
        SightConfig->LoseSightRadius = 2000.0f;
        SightConfig->PeripheralVisionAngleDegrees = 45.0f;
        SightConfig->SetMaxAge(5.0f);

        // 감지 대상 설정 (아군/적군/중립 모두)
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

        SightConfig->PointOfViewBackwardOffset = 0.0f;
        SightConfig->NearClippingRadius = 0.0f;

        EnemyPerceptionComponent->ConfigureSense(*SightConfig);
        EnemyPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
    }

    PrimaryActorTick.bCanEverTick = true;
}

void ALA_EnemyController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UBlackboardComponent* BB = GetBlackboardComponent();
    APawn* MyPawn = GetPawn();

    if (BB && MyPawn)
    {
        // 1. 시야 감지 시스템이 찾아낸 타겟 액터를 블랙보드에서 긁어옵니다.
        AActor* Target = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor")));

        if (Target)
        {
            // 2. 플레이어와 나 사이의 실제 거리를 계산합니다.
            float Distance = FVector::Dist(MyPawn->GetActorLocation(), Target->GetActorLocation());

            // 3. 에너미의 근접 공격 사거리 세팅 (기획 수치에 맞게 150~250 사이로 조절하세요!)
            float EnemyAttackRange = 250.0f;

            if (Distance <= EnemyAttackRange)
            {
                // ⚔️ 사거리 안이라면 비헤비어 트리가 기다리는 스위치를 'Is Set' 상태로 채워줍니다!
                BB->SetValueAsObject(TEXT("IsTargetSpotted"), Target);
            }
            else
            {
                // 🏃 사거리 밖으로 도망치면 스위치를 꺼버려서(Not Set) 다시 쫓아가게 만듭니다.
                BB->ClearValue(TEXT("IsTargetSpotted"));
            }
        }
        else
        {
            // 타겟 자체가 사라지면 스위치도 당연히 청소합니다.
            BB->ClearValue(TEXT("IsTargetSpotted"));
        }
    }
}

void ALA_EnemyController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (BTAsset)
    {
        // Behavior Tree 실행
        if (RunBehaviorTree(BTAsset))
        {
            // 초기 위치 저장 (순찰 복귀용)
            GetBlackboardComponent()->SetValueAsVector(TEXT("HomeLocation"), InPawn->GetActorLocation());
        }
    }
}

void ALA_EnemyController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor) return;

    // [1단계 안전장치] 내가 나를 보거나, 내 동료 몬스터를 본 경우 무조건 0.001초 만에 칼차단
    if (Actor == this || Actor == GetPawn() || Actor->ActorHasTag(FName("Team.Enemy")) || Actor->IsA(ALA_EnemyCharacter::StaticClass()))
        {
            return;
        }

    // [2단계 안전장치] 게임플레이 태그 검사
    IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(Actor);
    FGameplayTagContainer TargetTags;
    if (TagInterface)
    {
        TagInterface->GetOwnedGameplayTags(TargetTags);
    }

    // [3단계 판정] 상대방이 플레이어 진영("Team.Ally")인지 2중으로 교차 검증
    bool bIsAlly = false;

    // 플레이어 캐릭터의 이름이나 태그 확인
    if (TargetTags.HasTag(FGameplayTag::RequestGameplayTag(FName("Team.Ally"))) ||
        Actor->ActorHasTag(FName("Team.Ally")) ||
        Actor->GetName().Contains(TEXT("Player")) ||
        Actor->GetName().Contains(TEXT("Ally")) ||                         // 이름에 Ally가 포함되어 있거나
        (Actor->GetClass()->GetName().Contains(TEXT("Ally"))) ||            // 블루프린트 클래스명에 Ally가 있거나
        Actor->IsA(ACharacter::StaticClass()) && !Actor->ActorHasTag(FName("Team.Enemy"))) // 에너미가 아닌 모든 캐릭터 계열 강제 포함
    {
        bIsAlly = true;
    }

    //  [4단계 가동] 확실하게 아군(플레이어)일 때만 블랙보드 갱신
    if (bIsAlly)
    {
        UBlackboardComponent* BB = GetBlackboardComponent();
        if (!BB) return;

        if (Stimulus.WasSuccessfullySensed())
        {
            // 시야에 플레이어가 들어오면 정확하게 블랙보드에 장착
            BB->SetValueAsObject(TEXT("TargetActor"), Actor);
        }
        else
        {
            // 시야에서 사라지면 현재 잡고 있던 타겟이 맞는지 확인 후 해제
            if (BB->GetValueAsObject(TEXT("TargetActor")) == Actor)
            {
                BB->ClearValue(TEXT("TargetActor"));
            }
        }
    }
}
