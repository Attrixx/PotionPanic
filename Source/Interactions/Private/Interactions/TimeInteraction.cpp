#include "Interactions/TimeInteraction.h"
#include "InteractionSettings/TimeInteractionSetting.h"

void UTimeInteraction::Init(UInteractionSetting* Settings)
{
	UTimeInteractionSetting* TimeSettings = Cast<UTimeInteractionSetting>(Settings);
	
	SecondsToWait = TimeSettings->SecondsToWait;
}

void UTimeInteraction::StartInteraction(const FInteractionContext& Context)
{
	if (UWorld* World = GetOuter()->GetWorld())
	{
		FTimerHandle Handle;
		World->GetTimerManager().SetTimer(Handle, [Context]
		{
			FInteractionOutput Output;
			Output.InteractionResult = EInteractionResult::Success;	
			Context.OnInteractionFinished.Broadcast(Output);			
		}, SecondsToWait, false);
	}
}
