// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectOperatorCharacter.h"
#include "FPCharacter.generated.h"

class UInteractionComponent;
class UInputAction;

/**
 * First-person character for The Operator
 * Handles movement and sprint, delegates interaction to InteractionComponent
 */
UCLASS()
class PROJECTOPERATOR_API AFPCharacter : public AProjectOperatorCharacter
{
	GENERATED_BODY()

protected:
	/** Interaction component handles all interaction logic */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UInteractionComponent* InteractionComponent;

	/** Input action for interacting */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* InteractAction;

	/** Input action for sprinting */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* SprintAction;

	/** Sprint speed multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = 1.0, ClampMax = 3.0))
	float SprintSpeedMultiplier = 1.5f;

	/** Is the character currently sprinting */
	bool bIsSprinting = false;

	/** Base walk speed (stored from movement component) */
	float BaseWalkSpeed = 600.0f;

public:
	AFPCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** Start sprinting */
	void DoStartSprint();

	/** Stop sprinting */
	void DoEndSprint();
};