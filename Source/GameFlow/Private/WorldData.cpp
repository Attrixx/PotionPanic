// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldData.h"

#include "Misc/DataValidation.h"
#include "Rounds/RoundTree.h"

EDataValidationResult UWorldData::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);
	
	if (!Rounds)
	{
		Context.AddError(FText::FromString("Rounds asset is not set"));
		Result = EDataValidationResult::Invalid;
	}
	else
	{
		Result = CombineDataValidationResults(Result, Cast<const UPrimaryDataAsset>(Rounds)->IsDataValid(Context));
	}
	
	return Result;
}
