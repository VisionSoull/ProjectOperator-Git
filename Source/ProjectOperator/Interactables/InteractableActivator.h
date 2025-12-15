// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactables/InteractableBase.h"
#include "Interactables/EInteractionType.h"
#include "InteractableActivator.generated.h"

class IActivable;
class AActivableBase;

/**
 * Interactable that can activate other activables when interacted with
 * Examples: buttons, levers, switches, pressure plates
 * Contains all timing logic (hold, cooldown) and provides manual activation functions
 */
UCLASS(Abstract, Blueprintable)
class PROJECTOPERATOR_API AInteractableActivator : public AInteractableBase
{
	GENERATED_BODY()

protected:
	/** How this interactable responds to interaction input */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	EInteractionType InteractionType = EInteractionType::Instant;

	/** Use a custom cooldown instead of the default from settings */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	bool bUseCustomCooldown = false;

	/** Custom cooldown time (only used if bUseCustomCooldown is true) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", meta = (EditCondition = "bUseCustomCooldown", ClampMin = 0.0, ClampMax = 10.0))
	float CustomCooldownDuration = 0.5f;

	/** Use a custom hold duration instead of the default from settings */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (EditCondition = "InteractionType == EInteractionType::Hold"))
	bool bUseCustomHoldDuration = false;

	/** Custom hold duration (only used if bUseCustomHoldDuration is true) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", meta = (EditCondition = "bUseCustomHoldDuration && InteractionType == EInteractionType::Hold", ClampMin = 0.1, ClampMax = 10.0))
	float CustomHoldDuration = 2.0f;

	/** Is this interactable currently busy and unable to accept new commands */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", ReplicatedUsing = OnRep_IsBusy)
	bool bIsBusy = false;

	/** Can this interactable be used multiple times, or only once? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	bool bCanBeUsedMultipleTimes = true;

	/** Should this activator control multiple activables? */
	UPROPERTY(EditAnywhere, Category = "Activation")
	bool bActivateMultipleActivables = false;

	/** Single activable to control (only if bActivateMultipleActivables is false) */
	UPROPERTY(EditAnywhere, Category = "Activation", meta = (EditCondition = "!bActivateMultipleActivables", EditConditionHides))
	AActivableBase* TargetActivable = nullptr;

	/** Multiple activables to control (only if bActivateMultipleActivables is true) */
	UPROPERTY(EditAnywhere, Category = "Activation", meta = (EditCondition = "bActivateMultipleActivables", EditConditionHides))
	TArray<AActivableBase*> TargetActivables;

	/** Is this interactable currently active/on (for toggle types) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Replicated)
	bool bIsActive = false;

	/** Is this interactable currently on cooldown */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", ReplicatedUsing = OnRep_IsOnCooldown)
	bool bIsOnCooldown = false;

	/** Has this interactable been used (for single-use interactables) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Replicated)
	bool bHasBeenUsed = false;

	/** Actual cooldown used at runtime */
	UPROPERTY(VisibleAnywhere, Category = "Interaction")
	float CooldownDuration = 0.5f;

	/** Actual hold duration used at runtime */
	UPROPERTY(VisibleAnywhere, Category = "Interaction")
	float RequiredHoldDuration = 2.0f;

	/** Timer for hold duration tracking */
	FTimerHandle HoldTimer;

	/** Timer for cooldown */
	FTimerHandle CooldownTimer;

	/** Character currently holding interaction */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", ReplicatedUsing = OnRep_HoldingCharacter)
	ACharacter* HoldingCharacter = nullptr;

public:
	AInteractableActivator();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Override interaction methods - DO NOT override these in Blueprint
	virtual void BeginInteract_Implementation(ACharacter* Character) override final;
	virtual void EndInteract_Implementation(ACharacter* Character) override final;
	virtual void BeginFocus_Implementation(ACharacter* Character, UPrimitiveComponent* FocusedComponent) override final;
	virtual bool CanInteract_Implementation(ACharacter* Character) const override;

	/** Called when hold duration completes */
	void OnHoldCompleted();

	/** Called when cooldown timer expires */
	void OnCooldownExpired();

	/** Executes the interaction logic based on type */
	void ExecuteInteraction(ACharacter* Character);

	/** Updates the visual feedback based on current focus and cooldown state */
	virtual void UpdateFocusVisual() override;

	// Blueprint Events (extension points)

	/** Called before interaction validation - use for pre-interaction effects */
	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction", meta = (DisplayName = "Before Interact"))
	void OnBeforeInteract(ACharacter* Character);

	/** Called when interaction successfully completes - override for custom behavior */
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction", meta = (DisplayName = "Interacted"))
	void OnInteracted(ACharacter* Character);

	/** Multicast to notify all clients when interaction completes */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnInteracted(ACharacter* Character);

	/** Called when hold interaction starts */
	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction", meta = (DisplayName = "Hold Started"))
	void OnHoldStarted(ACharacter* Character);

	/** Called when hold interaction is cancelled before completion */
	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction", meta = (DisplayName = "Hold Cancelled"))
	void OnHoldCancelled(ACharacter* Character);

	/** Called when toggle state changes (only for Toggle type) */
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction", meta = (DisplayName = "Toggle Changed"))
	void OnToggleChanged(bool bNewState, ACharacter* Character);

	// Blueprint callable helpers

	/** Set busy state - when true, blocks new activations */
	UFUNCTION(BlueprintCallable, Category = "Activation")
	void SetBusy(bool bNewBusy);

	/** Returns whether this activable is currently busy */
	UFUNCTION(BlueprintPure, Category = "Activation")
	bool IsBusy() const { return bIsBusy; }

public:
	// Activation Functions (Blueprint calls these manually)

	/** Activate all target activable(s) */
	UFUNCTION(BlueprintCallable, Category = "Activation")
	void ActivateTargets();

	/** Deactivate all target activable(s) */
	UFUNCTION(BlueprintCallable, Category = "Activation")
	void DeactivateTargets();

	/** Toggle all target activable(s) */
	UFUNCTION(BlueprintCallable, Category = "Activation")
	void ToggleTargets();

	// Helper Functions

	/** Get the single target activable (if not using multiple) */
	UFUNCTION(BlueprintPure, Category = "Activation")
	AActivableBase* GetTargetActivable() const { return TargetActivable; }

	/** Get all target activables (if using multiple) */
	UFUNCTION(BlueprintPure, Category = "Activation")
	TArray<AActivableBase*> GetTargetActivables() const { return TargetActivables; }

	/** Manually set the active state (for Toggle types) */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetActiveState(bool bNewState);

	/** Reset this interactable (clears used state, cooldown, etc.) */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void ResetInteractable();

	/** Get current hold progress (0-1 range, only valid during hold) */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	float GetHoldProgress() const;

	/** Get the required hold duration for this interactable */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	float GetRequiredHoldDuration();

	/** Get the cooldown duration for this interactable */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	float GetCooldownDuration();

	/** Returns current active state */
	UFUNCTION(BlueprintPure, Category = "State")
	bool IsActive() const { return bIsActive; }

protected:
	// Replication functions

	UFUNCTION()
	void OnRep_IsOnCooldown();

	UFUNCTION()
	void OnRep_IsBusy();

	UFUNCTION()
	void OnRep_HoldingCharacter();
};