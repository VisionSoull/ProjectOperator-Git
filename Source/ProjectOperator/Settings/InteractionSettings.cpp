// Fill out your copyright notice in the Description page of Project Settings.


#include "Settings/InteractionSettings.h"

UInteractionSettings* UInteractionSettings::GetInteractionSettings()
{
	return GetMutableDefault<UInteractionSettings>();
}

UMaterialInterface* UInteractionSettings::GetOutlineMaterialLoaded() const
{
	if (!OutlineMaterial.IsNull())
	{
		return Cast<UMaterialInterface>(OutlineMaterial.TryLoad());
	}
	return nullptr;
}