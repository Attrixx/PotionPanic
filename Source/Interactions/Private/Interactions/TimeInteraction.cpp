#include "Interactions/TimeInteraction.h"
#include "InteractionSettings/TimeInteractionSetting.h"

DEFINE_LOG_CATEGORY_STATIC(MS_TimeInteraction, Verbose, All);

void UTimeInteraction::Init(UInteractionSettingBase* Settings)
{
	UTimeInteractionSetting* TimeSettings = CastChecked<UTimeInteractionSetting>(Settings);
	
	SecondsToWait = TimeSettings->SecondsToWait;
	check(SecondsToWait >= 0.f);
}

void UTimeInteraction::StartInteraction(const FInteractionContext& Context)
{
	if (UWorld* World = GetOuter()->GetWorld())
	{
		FTimerHandle Handle;
		UE_LOG(MS_TimeInteraction, Verbose, TEXT("Started waiting on Time Interaction"));
		World->GetTimerManager().SetTimer(Handle, [Context]
		{
			FInteractionOutput Output;
			UE_LOG(MS_TimeInteraction, Verbose, TEXT("Stopped waiting on Time Interaction"));			
			Output.InteractionResult = EInteractionResult::Success;	
			Context.OnInteractionFinished.Broadcast(Output);			
		}, SecondsToWait, false);
	}
}
