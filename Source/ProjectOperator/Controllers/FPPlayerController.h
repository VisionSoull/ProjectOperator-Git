// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FPPlayerController.generated.h"

class UInputMappingContext;

/**
 * Player controller for the first-person Operator player
 * Manages input mapping contexts and FP-specific input handling
 */
UCLASS()
class PROJECTOPERATOR_API AFPPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	/** Input Mapping Context for FP controls */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

public:
	AFPPlayerController();

protected:
	virtual void SetupInputComponent() override;
};