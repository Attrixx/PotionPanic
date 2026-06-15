// Fill out your copyright notice in the Description page of Project Settings.

#include "AlchemistController.h"
#include <EngineUtils.h>

DEFINE_LOG_CATEGORY_STATIC(MS_AlchemistController, Log, All);

AAlchemistController::AAlchemistController()
{
	bAutoManageActiveCameraTarget = false;
}

void AAlchemistController::BeginPlay()
{
	Super::BeginPlay();
	
	if (AActor* ViewTarget = FindViewTarget())
	{
		SetViewTarget(ViewTarget);
	}
	else
	{
		UE_LOGFMT(MS_AlchemistController, Error, "Could not find view target actor with tag '{0}'", ViewTargetTag);
	}
}

AActor* AAlchemistController::FindViewTarget() const
{
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		if (It->ActorHasTag(ViewTargetTag))
		{
			return *It;
		}
	}
	return nullptr;
}
