// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/InteractionComponent.h"
#include "Components/SphereComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Interactables/Interactable.h"
#include "Interactables/InteractableBase.h"
#include "DrawDebugHelpers.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Create interaction zone sphere
	InteractionZone = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionZone"));
	InteractionZone->SetSphereRadius(InteractionRadius);
	InteractionZone->SetCollisionProfileName("InteractionZone");
	InteractionZone->SetComponentTickEnabled(false);
}

void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache owner character reference
	OwnerCharacter = Cast<ACharacter>(GetOwner());

	// Attach interaction zone to owner's root
	if (AActor* Owner = GetOwner())
	{
		InteractionZone->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		InteractionZone->RegisterComponent();
	}

	// Bind overlap events
	InteractionZone->OnComponentBeginOverlap.AddDynamic(this, &UInteractionComponent::OnInteractionZoneBeginOverlap);
	InteractionZone->OnComponentEndOverlap.AddDynamic(this, &UInteractionComponent::OnInteractionZoneEndOverlap);

}

void UInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clear interaction timer
	if (InteractionCheckTimer.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(InteractionCheckTimer);
	}

	// Clear current interactable focus
	if (CurrentInteractable.GetInterface())
	{
		IInteractable::Execute_EndFocus(CurrentInteractable.GetObject(), OwnerCharacter, CurrentHitComponent);
	}

	Super::EndPlay(EndPlayReason);
}

void UInteractionComponent::OnInteractionZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == GetOwner()) return;

	// Check if actor implements IInteractable
	if (OtherActor->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
	{
		TScriptInterface<IInteractable> Interactable;
		Interactable.SetObject(OtherActor);
		Interactable.SetInterface(Cast<IInteractable>(OtherActor));

		NearbyInteractables.Add(Interactable);

		// Start checking for aimed interactable if this is the first one
		if (NearbyInteractables.Num() == 1)
		{
			GetWorld()->GetTimerManager().SetTimer(InteractionCheckTimer, this, &UInteractionComponent::CheckAimedInteractable,
				InteractionCheckInterval, true);
		}
	}
}

void UInteractionComponent::OnInteractionZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor || OtherActor == GetOwner()) return;

	// Remove from nearby interactables
	NearbyInteractables.RemoveAll([OtherActor](const TScriptInterface<IInteractable>& Interactable)
		{
			return Interactable.GetObject() == OtherActor;
		});

	// If this was our current interactable, clear it
	if (CurrentInteractable.GetObject() == OtherActor)
	{
		UpdateCurrentInteractable(TScriptInterface<IInteractable>(), nullptr);
	}

	// Stop checking if no more candidates
	if (NearbyInteractables.Num() == 0)
	{
		GetWorld()->GetTimerManager().ClearTimer(InteractionCheckTimer);
	}
}

void UInteractionComponent::CheckAimedInteractable()
{
	if (NearbyInteractables.Num() == 0)
	{
		UpdateCurrentInteractable(TScriptInterface<IInteractable>(), nullptr);
		return;
	}

	// Need valid trace origin
	if (!TraceOrigin)
	{
		return;
	}

	// Get trace direction
	FVector Start = TraceOrigin->GetComponentLocation();
	FVector Direction = bUseCustomDirection
		? CustomTraceDirection.GetSafeNormal()
		: TraceOrigin->GetForwardVector();
	FVector End = Start + (Direction * TraceDistance);

	// Perform sphere sweep
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());

	bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		Start,
		End,
		FQuat::Identity,
		InteractableChannel,
		FCollisionShape::MakeSphere(TraceSphereRadius),
		QueryParams
	);

	// Check if we hit an interactable that's in our nearby list
	TScriptInterface<IInteractable> NewInteractable;
	UPrimitiveComponent* HitComponent = nullptr;
	if (bHit && HitResult.GetActor())
	{
		for (const TScriptInterface<IInteractable>& Candidate : NearbyInteractables)
		{
			if (Candidate.GetObject() == HitResult.GetActor())
			{
				NewInteractable = Candidate;
				HitComponent = HitResult.GetComponent();
				break;
			}
		}
	}

	// Update if changed
	if (NewInteractable.GetObject() != CurrentInteractable.GetObject() || HitComponent != CurrentHitComponent)
	{
		UpdateCurrentInteractable(NewInteractable, HitComponent);
	}
}

void UInteractionComponent::UpdateCurrentInteractable(TScriptInterface<IInteractable> NewInteractable, UPrimitiveComponent* HitComponent)
{
	// Remove focus from previous interactable
	if (CurrentInteractable.GetInterface() && CurrentHitComponent)
	{
		IInteractable::Execute_EndFocus(CurrentInteractable.GetObject(), OwnerCharacter, CurrentHitComponent);
	}

	// Update current
	CurrentInteractable = NewInteractable;
	CurrentHitComponent = HitComponent;

	// Give focus to new interactable
	if (CurrentInteractable.GetInterface() && CurrentHitComponent)
	{
		IInteractable::Execute_BeginFocus(CurrentInteractable.GetObject(), OwnerCharacter, CurrentHitComponent);
	}
}

void UInteractionComponent::DoInteractPressed()
{
	if (CurrentInteractable.GetInterface())
	{
		AInteractableBase* Interactable = Cast<AInteractableBase>(CurrentInteractable.GetObject());

		if (!Interactable)
		{
			return;
		}

		if (OwnerCharacter->HasAuthority())
		{
			IInteractable::Execute_BeginInteract(Interactable, OwnerCharacter);
		}
		else
		{
			Server_RequestInteract(Interactable, OwnerCharacter);
		}
	}
}

void UInteractionComponent::DoInteractReleased()
{
	if (CurrentInteractable.GetInterface())
	{
		AInteractableBase* Interactable = Cast<AInteractableBase>(CurrentInteractable.GetObject());

		if (!Interactable)
		{
			return;
		}

		if (OwnerCharacter->HasAuthority())
		{
			IInteractable::Execute_EndInteract(Interactable, OwnerCharacter);
		}
		else
		{
			Server_RequestEndInteract(Interactable, OwnerCharacter);
		}
	}
}

void UInteractionComponent::Server_RequestInteract_Implementation(AInteractableBase* Interactable, ACharacter* Character)
{
	if (Interactable)
	{
		IInteractable::Execute_BeginInteract(Interactable, Character);
	}
}

void UInteractionComponent::Server_RequestEndInteract_Implementation(AInteractableBase* Interactable, ACharacter* Character)
{
	if (Interactable)
	{
		IInteractable::Execute_EndInteract(Interactable, Character);
	}
}