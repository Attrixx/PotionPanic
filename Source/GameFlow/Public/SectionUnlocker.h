// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SectionUnlocker.generated.h"

class USectionAsset;

DECLARE_MULTICAST_DELEGATE_OneParam(FSectionDelegate, USectionAsset&);

/**
 * 
 */
UCLASS()
class GAMEFLOW_API USectionUnlocker : public UWorldSubsystem
{
	GENERATED_BODY()
	
	FSectionDelegate OnSectionUnlocked;
};
