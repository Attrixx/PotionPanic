// Fill out your copyright notice in the Description page of Project Settings.

#include "AlchemistController.h"
#include "InputBindable.h"
#include <EngineUtils.h>
#include <EnhancedInputComponent.h>

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

void AAlchemistController::SetupInputComponent()
{
	Super::SetupInputComponent();
		
	auto* EIC = CastChecked<UEnhancedInputComponent>(InputComponent);
	for (auto* Component : GetComponents())
	{
		if (Component && Component->Implements<UInputBindable>())
		{
			IInputBindable::Execute_SetupInputComponent(Component, EIC);
		}
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
