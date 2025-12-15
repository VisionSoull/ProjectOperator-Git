// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/FPGameMode.h"
#include "Characters/FPCharacter.h"
#include "Controllers/FPPlayerController.h"

AFPGameMode::AFPGameMode()
{
	// Set default pawn and controller for FP testing
	DefaultPawnClass = AFPCharacter::StaticClass();
	PlayerControllerClass = AFPPlayerController::StaticClass();
}

void AFPGameMode::BeginPlay()
{
	Super::BeginPlay();
}