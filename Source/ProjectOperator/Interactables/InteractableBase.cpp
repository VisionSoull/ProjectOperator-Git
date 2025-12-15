// Fill out your copyright notice in the Description page of Project Settings.

#include "Interactables/InteractableBase.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Settings/InteractionSettings.h"

AInteractableBase::AInteractableBase()
{
	PrimaryActorTick.bCanEverTick = false;

	// Enable replication
	bReplicates = true;

	// Create root scene component
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// Create interactable mesh
	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	BaseMesh->SetupAttachment(RootComponent);
	BaseMesh->SetCollisionProfileName("Interactable");
	BaseMesh->SetComponentTickEnabled(false);
}

void AInteractableBase::BeginPlay()
{
	Super::BeginPlay();

	// Load settings and cache outline material
	const UInteractionSettings* Settings = GetDefault<UInteractionSettings>();
	if (!Settings->OutlineMaterial.IsNull())
	{
		CachedOutlineMaterial = Cast<UMaterialInterface>(Settings->OutlineMaterial.TryLoad());
	}
}

void AInteractableBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// Derived classes add their replicated properties
}

void AInteractableBase::BeginInteract_Implementation(ACharacter* Character)
{
	// Default: empty
	// Simple interactables (food, climbable) override this in Blueprint
}

void AInteractableBase::EndInteract_Implementation(ACharacter* Character)
{
	// Default: empty
	// Simple interactables override this in Blueprint if needed
}

void AInteractableBase::BeginFocus_Implementation(ACharacter* Character, UPrimitiveComponent* FocusedComponent)
{
	bIsFocused = true;
	CurrentlyFocusedComponent = FocusedComponent;
	CurrentlyFocusingCharacter = Character;
	UpdateFocusVisual();
}

void AInteractableBase::EndFocus_Implementation(ACharacter* Character, UPrimitiveComponent* FocusedComponent)
{
	bIsFocused = false;

	// Clear outline if it was a mesh component
	if (UMeshComponent* MeshComp = Cast<UMeshComponent>(CurrentlyFocusedComponent))
	{
		MeshComp->SetOverlayMaterial(nullptr);
	}
	CurrentlyFocusedComponent = nullptr;
	CurrentlyFocusingCharacter = nullptr;
}

bool AInteractableBase::CanInteract_Implementation(ACharacter* Character) const
{
	// Default: always can interact
	// Derived classes override for specific conditions
	return true;
}

void AInteractableBase::UpdateFocusVisual()
{
	// Only show outline if focused AND can interact
	bool bShouldShowOutline = bIsFocused && CanInteract_Implementation(CurrentlyFocusingCharacter);

	// Only apply outline if the focused component is a mesh
	if (UMeshComponent* MeshComp = Cast<UMeshComponent>(CurrentlyFocusedComponent))
	{
		MeshComp->SetOverlayMaterial(bShouldShowOutline ? CachedOutlineMaterial : nullptr);
	}
}