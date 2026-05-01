// Fill out your copyright notice in the Description page of Project Settings.


#include "LA_DefaultPlayerController.h"
#include "EnhancedInputSubsystems.h"

void ALA_DefaultPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 플레이어 컨트롤러가 Local 플레이어의 컨트롤러인지 확인
	if (IsLocalPlayerController())
	{
		// 향상된 입력의 하위 시스템 얻기
		if (UEnhancedInputLocalPlayerSubsystem* subSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			// 키 입력 설정을 할당된 InputMappingContext 에셋으로 사용
			if (MappingContext != nullptr)
			{
				subSystem->AddMappingContext(MappingContext, 0);
			}
		}
	}
}
