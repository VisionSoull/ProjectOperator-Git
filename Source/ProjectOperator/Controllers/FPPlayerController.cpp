// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/FPPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"

AFPPlayerController::AFPPlayerController()
{
	// Future: Add FP-specific settings
}

void AFPPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Add Input Mapping Context for local player
	if (IsLocalPlayerController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
}