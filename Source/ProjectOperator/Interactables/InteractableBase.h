// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactables/Interactable.h"
#include "InteractableBase.generated.h"

class UStaticMeshComponent;
class ACharacter;

/**
 * Minimal base class for all interactable objects in The Operator
 * Provides focus/outline and basic interaction contract
 * Specific interaction behaviors implemented in derived classes
 */
UCLASS(Abstract, Blueprintable)
class PROJECTOPERATOR_API AInteractableBase : public AActor, public IInteractable
{
	GENERATED_BODY()

protected:
	/** Visual mesh for the interactable */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* BaseMesh;

	/** Is this interactable currently being focused/aimed at? */
	bool bIsFocused = false;

	/** The component currently receiving focus */
	UPROPERTY(Transient)
	UPrimitiveComponent* CurrentlyFocusedComponent = nullptr;

	/** The character currently focusing on this interactable */
	UPROPERTY(Transient)
	ACharacter* CurrentlyFocusingCharacter = nullptr;

	/** Cached outline material loaded from settings */
	UPROPERTY(Transient)
	UMaterialInterface* CachedOutlineMaterial = nullptr;

public:
	AInteractableBase();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// IInteractable interface
	virtual void BeginInteract_Implementation(ACharacter* Character) override;
	virtual void EndInteract_Implementation(ACharacter* Character) override;
	virtual void BeginFocus_Implementation(ACharacter* Character, UPrimitiveComponent* FocusedComponent) override;
	virtual void EndFocus_Implementation(ACharacter* Character, UPrimitiveComponent* FocusedComponent) override;
	virtual bool CanInteract_Implementation(ACharacter* Character) const override;

	/** Updates the visual feedback based on current focus state */
	virtual void UpdateFocusVisual();

public:
	/** Returns the interactable mesh */
	UStaticMeshComponent* GetInteractableMesh() const { return BaseMesh; }
};