// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

class IInteractable;
class USphereComponent;
class UInputAction;

/**
 * Component that handles interaction detection and execution
 * Supports both sphere overlap detection and aimed trace detection
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTOPERATOR_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** Component to trace from (camera, hand, etc.) - set by owning character */
	UPROPERTY(BlueprintReadWrite, Category = "Interaction")
	USceneComponent* TraceOrigin = nullptr;

	/** Distance to trace from origin */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", meta = (ClampMin = 50.0, ClampMax = 1000.0))
	float TraceDistance = 300.0f;

	/** Sphere radius for trace forgiveness */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", meta = (ClampMin = 1.0, ClampMax = 50.0))
	float TraceSphereRadius = 5.0f;

	/** Use custom trace direction instead of TraceOrigin's forward vector */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bUseCustomDirection = false;

	/** Custom trace direction (only used if bUseCustomDirection is true) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", meta = (EditCondition = "bUseCustomDirection"))
	FVector CustomTraceDirection = FVector::ForwardVector;

protected:
	/** Cahced reference to owning character */
	UPROPERTY()
	ACharacter* OwnerCharacter = nullptr;

	/** Sphere for detecting nearby interactables */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* InteractionZone;

	/** Radius of interaction detection sphere */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (ClampMin = 50.0, ClampMax = 1000.0))
	float InteractionRadius = 250.0f;

	/** How often to check for aimed interactable (in seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (ClampMin = 0.01, ClampMax = 0.5))
	float InteractionCheckInterval = 0.1f;

	/** Collision channel to detect interactables */
	UPROPERTY(EditAnywhere, Category = "Interaction")
	TEnumAsByte<ECollisionChannel> InteractableChannel = ECC_GameTraceChannel2;

	/** Input action for interaction */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* InteractAction;

	/** List of nearby interactables */
	TArray<TScriptInterface<IInteractable>> NearbyInteractables;

	/** Currently aimed interactable */
	TScriptInterface<IInteractable> CurrentInteractable;

	/** The component currently being aimed at */
	UPROPERTY()
	UPrimitiveComponent* CurrentHitComponent = nullptr;

	/** Timer for checking aimed interactable */
	FTimerHandle InteractionCheckTimer;

public:
	UInteractionComponent();

	/** Called when interact button is pressed - call from Character's SetupPlayerInputComponent */
	void DoInteractPressed();

	/** Called when interact button is released - call from Character's SetupPlayerInputComponent */
	void DoInteractReleased();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Called when interaction zone overlaps with an actor */
	UFUNCTION()
	void OnInteractionZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Called when interaction zone stops overlapping with an actor */
	UFUNCTION()
	void OnInteractionZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** Check which interactable is currently aimed at */
	void CheckAimedInteractable();

	/** Update the current interactable and handle outline visibility */
	void UpdateCurrentInteractable(TScriptInterface<IInteractable> NewInteractable, UPrimitiveComponent* HitComponent);

	 /** Server RPC to request interaction */
    UFUNCTION(Server, Reliable)
    void Server_RequestInteract(AInteractableBase* Interactable, ACharacter* Character);

    /** Server RPC to request end interaction */
    UFUNCTION(Server, Reliable)
    void Server_RequestEndInteract(AInteractableBase* Interactable, ACharacter* Character);
};