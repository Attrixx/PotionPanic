// Fill out your copyright notice in the Description page of Project Settings.

#include "Interactions/IFTInteraction.h"
#include "InteractionSettings/IFTInteractionSetting.h"

DEFINE_LOG_CATEGORY_STATIC(MS_ITFInteraction, Log, All);

void UIFTInteraction::Init(UInteractionSetting* Settings)
{
	UIFTInteractionSetting* TimeSettings = Cast<UIFTInteractionSetting>(Settings);
	
	SecondsBeforeWindow = TimeSettings->SecondsBeforeWindow;
	WindowLengthSeconds = TimeSettings->WindowLengthSeconds;
}

void UIFTInteraction::StartInteraction(const FInteractionContext& Context)
{
	InteractionContext = Context;
	if (UWorld* World = GetOuter()->GetWorld())
	{
		FTimerHandle Handle;
		Status = EIFTStatus::WaitingForWindow;
		UE_LOGFMT(MS_ITFInteraction, Log, "Waiting for Window opening");
		
		World->GetTimerManager().SetTimer(Handle, [&, World]
		{
			Status = EIFTStatus::DuringWindow;
			UE_LOGFMT(MS_ITFInteraction, Log, "Window opened, Waiting for User interaction");
			
			World->GetTimerManager().SetTimer(WindowHandle, [&]
			{
				UE_LOGFMT(MS_ITFInteraction, Log, "Missed Window");				
				InteractionOutput.InteractionResult = EInteractionResult::Fail;
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
		UE_LOGFMT(MS_ITFInteraction, Log, "Window interaction triggered successfully");
		InteractionOutput.InteractionResult = EInteractionResult::Success;
		WindowHandle.Invalidate();
		InteractionContext.OnInteractionFinished.Broadcast(InteractionOutput);
	}
}
