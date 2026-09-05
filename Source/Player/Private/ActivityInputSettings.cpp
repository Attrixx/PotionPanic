// Fill out your copyright notice in the Description page of Project Settings.

#include "ActivityInputSettings.h"
#include <InputAction.h>

FName UActivityInputSettings::GetCategoryName() const
{
	return TEXT("Game");
}

UInputAction* UActivityInputSettings::GetSlotAction(EActivityInputSlot Slot) const
{
	const TSoftObjectPtr<UInputAction>* Found = SlotActions.Find(Slot);
	return Found ? Found->LoadSynchronous() : nullptr;
}
