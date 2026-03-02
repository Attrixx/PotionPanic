// Fill out your copyright notice in the Description page of Project Settings.

#include "Interactions/InteractionTest.h"
#include "Logging/StructuredLog.h"

DEFINE_LOG_CATEGORY_STATIC(PP_InteractionTest, Log, All);

bool UInteractionTest::StartTestInteraction(const UInteractionDefinitionAsset *InteractionDefinition)
{
	const bool bStarted = StartInteraction(InteractionDefinition);
	if (bStarted)
	{
		UE_LOGFMT(MS_InteractionTest, Log, "Interaction test started.");
	}

	return bStarted;
}
