// Fill out your copyright notice in the Description page of Project Settings.

#include "Interactions/IFTInteraction.h"
#include "InteractionSettings/IFTInteractionSetting.h"

DEFINE_LOG_CATEGORY_STATIC(MS_ITFInteraction, Verbose, All);

void UIFTInteraction::Init(UInteractionSettingBase* Settings)
{
	UIFTInteractionSetting* TimeSettings = CastChecked<UIFTInteractionSetting>(Settings);
	
	SecondsBeforeWindow = TimeSettings->SecondsBeforeWindow;
	WindowLengthSeconds = TimeSettings->WindowLengthSeconds;
	check(SecondsBeforeWindow >= 0.f);
	check(WindowLengthSeconds >= 0.f);
}

void UIFTInteraction::StartInteraction(const FInteractionContext& Context)
{
	InteractionContext = Context;
	if (UWorld* World = GetOuter()->GetWorld())
	{
		FTimerHandle Handle;
		Status = EIFTStatus::WaitingForWindow;
		UE_LOGFMT(MS_ITFInteraction, Verbose, "Waiting for Window opening");
		
		World->GetTimerManager().SetTimer(Handle, [&, World]
		{
			Status = EIFTStatus::DuringWindow;
			UE_LOGFMT(MS_ITFInteraction, Verbose, "Window opened, Waiting for User interaction");
			
			World->GetTimerManager().SetTimer(WindowHandle, [&]
			{
				Status = EIFTStatus::PastWindow;
				UE_LOGFMT(MS_ITFInteraction, Verbose, "Missed Window");				
				InteractionOutput.InteractionResult = EInteractionResult::Fail;
				// TODO François Compute interaction score
				InteractionContext.OnInteractionFinished.Broadcast(InteractionOutput);
			}, WindowLengthSeconds, false);
			
		}, SecondsBeforeWindow, false);
	}
}

void UIFTInteraction::InteractWhileProcess()
{
	Super::InteractWhileProcess();
	
	if (Status == EIFTStatus::DuringWindow)
	{
		UE_LOGFMT(MS_ITFInteraction, Verbose, "Window interaction triggered successfully");
		GetWorld()->GetTimerManager().ClearTimer(WindowHandle);
		InteractionOutput.InteractionResult = EInteractionResult::Success;		
		InteractionContext.OnInteractionFinished.Broadcast(InteractionOutput);
	}
}
