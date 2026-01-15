// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomGameViewportClient.h"

bool UCustomGameViewportClient::InputKey(const FInputKeyEventArgs& EventArgs)
{
	if (EventArgs.Event == EInputEvent::IE_Pressed && EventArgs.IsGamepad())
	{
		bool bIsAssigned = false;
		if (UWorld* CurrentWorld = GetWorld())
		{
			for (auto It = CurrentWorld->GetGameInstance()->GetLocalPlayerIterator(); It; ++It)
			{
				if ((*It)->GetControllerId() == EventArgs.ControllerId)
				{
					if (GEngine)
					{
						GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("ControllerId %d is already assigned to a local player %s."), EventArgs.ControllerId, *(*It)->GetFullName()));
					}
					bIsAssigned = true;
					break;
				}
			}
		}

		// TODO: Avoid hardcoded keys ?
		if (!bIsAssigned && (EventArgs.Key == EKeys::Gamepad_FaceButton_Bottom || EventArgs.Key == EKeys::Gamepad_Special_Right))
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("InputKey received: ControllerId=%d, Key=%s"), EventArgs.ControllerId, *EventArgs.Key.ToString()));
			}
			OnLocalPlayerJoinRequest.Broadcast(EventArgs.ControllerId);

			// Consume input to avoid doing other things
			return true;
		}
	}

	return Super::InputKey(EventArgs);
}
