// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactions/InteractionTest.h"

DEFINE_LOG_CATEGORY_STATIC(PP_InteractionTest, Log, All);

void UInteractionTest::StartInteraction(const FInteractionContext& Context)
{
	UE_LOGFMT(PP_InteractionTest, Log, "Interaction test, started!");
	
	FInteractionOutput Output;
	Output.InteractionResult = EInteractionResult::Success;	
	Context.OnInteractionFinished.Broadcast(Output);
}
