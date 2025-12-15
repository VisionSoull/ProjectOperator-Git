// Fill out your copyright notice in the Description page of Project Settings.

#include "Interactables/InteractableActivator.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Settings/InteractionSettings.h"
#include "Activables/Activable.h"
#include "Activables/ActivableBase.h"

AInteractableActivator::AInteractableActivator()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AInteractableActivator::BeginPlay()
{
	Super::BeginPlay();
}

void AInteractableActivator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AInteractableActivator, bIsBusy);
	DOREPLIFETIME(AInteractableActivator, bIsActive);
	DOREPLIFETIME(AInteractableActivator, bIsOnCooldown);
	DOREPLIFETIME(AInteractableActivator, bHasBeenUsed);
	DOREPLIFETIME(AInteractableActivator, HoldingCharacter);
}

void AInteractableActivator::BeginInteract_Implementation(ACharacter* Character)
{
	OnBeforeInteract(Character);

	if (!CanInteract_Implementation(Character))
	{
		return;
	}
	switch (InteractionType)
	{
	case EInteractionType::Instant:
		ExecuteInteraction(Character);
		break;

	case EInteractionType::Hold:
		// Start hold timer
		HoldingCharacter = Character;
		GetWorldTimerManager().SetTimer(HoldTimer, this, &AInteractableActivator::OnHoldCompleted, RequiredHoldDuration, false);
		OnHoldStarted(Character);
		break;

	case EInteractionType::Toggle:
		ExecuteInteraction(Character);
		bIsActive = !bIsActive;
		OnToggleChanged(bIsActive, Character);
		break;

	case EInteractionType::UI:
		ExecuteInteraction(Character);
		break;
	}
}

void AInteractableActivator::EndInteract_Implementation(ACharacter* Character)
{
	// Only matters for Hold type
	if (InteractionType == EInteractionType::Hold)
	{
		// Only cancel if we're actually still holding
		if (HoldingCharacter != nullptr)
		{
			GetWorldTimerManager().ClearTimer(HoldTimer);
			OnHoldCancelled(Character);
			HoldingCharacter = nullptr;

			// Hold was cancelled, no cooldown triggered, keep outline visible if still focused
			UpdateFocusVisual();
		}
	}
}

void AInteractableActivator::BeginFocus_Implementation(ACharacter* Character, UPrimitiveComponent* FocusedComponent)
{
	Super::BeginFocus_Implementation(Character, FocusedComponent);
}

bool AInteractableActivator::CanInteract_Implementation(ACharacter* Character) const
{
	// Can't interact while busy
	if (bIsBusy)
	{
		return false;
	}

	// Can't interact if on cooldown
	if (bIsOnCooldown)
	{
		return false;
	}

	// Can't interact if already used and not reusable
	if (bHasBeenUsed && !bCanBeUsedMultipleTimes)
	{
		return false;
	}

	// Can't interact if currently being held by someone
	if (HoldingCharacter != nullptr && HoldingCharacter != Character)
	{
		return false;
	}

	return true;
}

void AInteractableActivator::OnHoldCompleted()
{
	if (HoldingCharacter)
	{
		ExecuteInteraction(HoldingCharacter);
		HoldingCharacter = nullptr;
	}
}

void AInteractableActivator::OnCooldownExpired()
{
	bIsOnCooldown = false;

	// Restore outline if still focused
	UpdateFocusVisual();
}
void AInteractableActivator::ExecuteInteraction(ACharacter* Character)
{
	// Mark as used if single-use
	if (!bCanBeUsedMultipleTimes)
	{
		bHasBeenUsed = true;
	}

	// Fire Blueprint event on ALL clients via multicast
	Multicast_OnInteracted(Character);

	// Start cooldown
	if (CooldownDuration > 0.0f)
	{
		bIsOnCooldown = true;

		// Hide outline when cooldown starts
		UpdateFocusVisual();

		GetWorldTimerManager().SetTimer(CooldownTimer, this, &AInteractableActivator::OnCooldownExpired, CooldownDuration, false);
	}
}

void AInteractableActivator::OnInteracted_Implementation(ACharacter* Character)
{
	// Default: empty
	// Blueprint overrides this to add sounds, VFX, and call ActivateTargets() when ready
}

void AInteractableActivator::Multicast_OnInteracted_Implementation(ACharacter* Character)
{
	OnInteracted(Character);
}

void AInteractableActivator::OnToggleChanged_Implementation(bool bNewState, ACharacter* Character)
{
	// Default: empty
	// Blueprint overrides this for custom toggle behavior
}

void AInteractableActivator::SetBusy(bool bNewBusy)
{
	bIsBusy = bNewBusy;

	UpdateFocusVisual();
}

void AInteractableActivator::UpdateFocusVisual()
{
	// Only show outline if focused AND not on cooldown AND can interact
	bool bShouldShowOutline = bIsFocused && CanInteract_Implementation(CurrentlyFocusingCharacter);

	if (UMeshComponent* MeshComp = Cast<UMeshComponent>(CurrentlyFocusedComponent))
	{
		MeshComp->SetOverlayMaterial(bShouldShowOutline ? CachedOutlineMaterial : nullptr);
	}
}

void AInteractableActivator::ActivateTargets()
{
	if (bActivateMultipleActivables)
	{
		// Activate all targets
		for (AActivableBase* Target : TargetActivables)
		{
			if (Target && Target->Implements<UActivable>())
			{
				IActivable::Execute_SetActive(Target, true);
			}
		}
	}
	else
	{
		// Activate single target
		if (TargetActivable && TargetActivable->Implements<UActivable>())
		{
			IActivable::Execute_SetActive(TargetActivable, true);
		}
	}
}

void AInteractableActivator::DeactivateTargets()
{
	if (bActivateMultipleActivables)
	{
		// Deactivate all targets
		for (AActivableBase* Target : TargetActivables)
		{
			if (Target && Target->Implements<UActivable>())
			{
				IActivable::Execute_SetActive(Target, false);
			}
		}
	}
	else
	{
		// Deactivate single target
		if (TargetActivable && TargetActivable->Implements<UActivable>())
		{
			IActivable::Execute_SetActive(TargetActivable, false);
		}
	}
}

void AInteractableActivator::ToggleTargets()
{
	if (bActivateMultipleActivables)
	{
		// Toggle all targets
		for (AActivableBase* Target : TargetActivables)
		{
			if (Target && Target->Implements<UActivable>())
			{
				IActivable::Execute_Toggle(Target);
			}
		}
	}
	else
	{
		// Toggle single target
		if (TargetActivable && TargetActivable->Implements<UActivable>())
		{
			IActivable::Execute_Toggle(TargetActivable);
		}
	}
}

void AInteractableActivator::SetActiveState(bool bNewState)
{
	if (bIsActive != bNewState)
	{
		bIsActive = bNewState;
		OnToggleChanged_Implementation(bIsActive, nullptr);
	}
}

void AInteractableActivator::ResetInteractable()
{
	bHasBeenUsed = false;
	bIsActive = false;
	bIsOnCooldown = false;
	HoldingCharacter = nullptr;
	bIsBusy = false;

	// Clear any active timers
	GetWorldTimerManager().ClearTimer(HoldTimer);
	GetWorldTimerManager().ClearTimer(CooldownTimer);

	// Update visuals
	UpdateFocusVisual();
}

float AInteractableActivator::GetHoldProgress() const
{
	if (InteractionType != EInteractionType::Hold || !HoldTimer.IsValid())
	{
		return 0.0f;
	}

	float Elapsed = GetWorldTimerManager().GetTimerElapsed(HoldTimer);
	return FMath::Clamp(Elapsed / RequiredHoldDuration, 0.0f, 1.0f);
}

float AInteractableActivator::GetRequiredHoldDuration()
{
	// Load settings
	const UInteractionSettings* Settings = GetDefault<UInteractionSettings>();

	// Hold duration: custom or default?
	return RequiredHoldDuration = bUseCustomHoldDuration ? CustomHoldDuration : Settings->DefaultHoldDuration;
}

float AInteractableActivator::GetCooldownDuration()
{
	// Load settings
	const UInteractionSettings* Settings = GetDefault<UInteractionSettings>();

	// Cooldown: custom or default?
	return CooldownDuration = bUseCustomCooldown ? CustomCooldownDuration : Settings->DefaultCooldown;
}

void AInteractableActivator::OnRep_IsOnCooldown()
{
	UpdateFocusVisual();
}

void AInteractableActivator::OnRep_IsBusy()
{
	UpdateFocusVisual();
}

void AInteractableActivator::OnRep_HoldingCharacter()
{
	UpdateFocusVisual();
}
