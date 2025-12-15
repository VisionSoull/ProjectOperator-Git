// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/FPCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "Components/InteractionComponent.h"

AFPCharacter::AFPCharacter()
{
	// Create interaction component
	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComponent"));
}

void AFPCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Store base walk speed
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		BaseWalkSpeed = MovementComp->MaxWalkSpeed;
	}

	// Configure interaction component to trace from camera
	if (InteractionComponent)
	{
		InteractionComponent->TraceOrigin = GetFirstPersonCameraComponent();
	}
}

void AFPCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Interact - delegate to InteractionComponent
		if (InteractAction && InteractionComponent)
		{
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, InteractionComponent, &UInteractionComponent::DoInteractPressed);
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, InteractionComponent, &UInteractionComponent::DoInteractReleased);
		}

		// Sprint
		if (SprintAction)
		{
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AFPCharacter::DoStartSprint);
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AFPCharacter::DoEndSprint);
		}
	}
}

void AFPCharacter::DoStartSprint()
{
	if (bIsSprinting) return;

	bIsSprinting = true;

	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->MaxWalkSpeed = BaseWalkSpeed * SprintSpeedMultiplier;
	}
}

void AFPCharacter::DoEndSprint()
{
	if (!bIsSprinting) return;

	bIsSprinting = false;

	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->MaxWalkSpeed = BaseWalkSpeed;
	}
}