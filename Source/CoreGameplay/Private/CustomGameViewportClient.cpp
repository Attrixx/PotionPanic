// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomGameViewportClient.h"
#include "InputMappingContext.h"
#include "InputAction.h"

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

		bool bIsJoinKey = false;
		// DefaultEngine.ini should define :
		// [/Script/CoreGameplay.CustomGameViewportClient]
		// JoinMappingContext=/Game/Path/To/Your/IMC_Default.IMC_Default
		// JoinAction=/Game/Path/To/Your/IA_Join.IA_Join
		UInputMappingContext* Context = JoinMappingContext.LoadSynchronous();
		UInputAction* Action = JoinAction.LoadSynchronous();

		if (Context && Action)
		{
			for (const FEnhancedActionKeyMapping& Mapping : Context->GetMappings())
			{
				if (Mapping.Action == Action && Mapping.Key == EventArgs.Key)
				{
					bIsJoinKey = true;
					break;
				}
			}
		}
		else
		{
			UE_LOG(MS_CustomGameViewportClient, Warning, TEXT("Input context is not properly defined in DefaultEngine.ini to detect new controller use."))
			// Fallback to hardcoded keys if no context/action is configured
			if (EventArgs.Key == EKeys::Gamepad_FaceButton_Bottom || EventArgs.Key == EKeys::Gamepad_Special_Right)
			{
				bIsJoinKey = true;
			}
		}



		if (!bIsAssigned && bIsJoinKey)
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

