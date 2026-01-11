// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ActivityRow.generated.h"

/**
 *
 */
USTRUCT()
struct COREGAMEPLAY_API FActivityRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText ActivityName;
};
