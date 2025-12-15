// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FPGameMode.generated.h"

/**
 * Game mode for first-person Operator testing and single-player FP gameplay
 * Sets up FP character and controller defaults
 */
UCLASS()
class PROJECTOPERATOR_API AFPGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFPGameMode();

protected:
	virtual void BeginPlay() override;
};
