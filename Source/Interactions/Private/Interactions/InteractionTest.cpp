// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactions/InteractionTest.h"

DEFINE_LOG_CATEGORY_STATIC(MS_InteractionTest, Verbose, All);

void UInteractionTest::Init(UInteractionSettingBase* Settings)
{
	// Nothing to do
}

void UInteractionTest::StartInteraction(const FInteractionContext& Context)
{
	UE_LOGFMT(MS_InteractionTest, Verbose, "Interaction test, started!");
	
	FInteractionOutput Output;
	Output.InteractionResult = EInteractionResult::Success;	
	Context.OnInteractionFinished.Broadcast(Output);
}
