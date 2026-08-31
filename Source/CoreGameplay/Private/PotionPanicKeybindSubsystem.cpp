#include "PotionPanicKeybindSubsystem.h"

#include "PotionPanicUserSettings.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Engine/LocalPlayer.h"

UInputMappingContext* UPotionPanicKeybindSubsystem::GetRuntimeContext(UInputMappingContext* SourceContext)
{
	if (!SourceContext)
	{
		return nullptr;
	}

	if (const TObjectPtr<UInputMappingContext>* Found = RuntimeContexts.Find(SourceContext))
	{
		return *Found;
	}

	UInputMappingContext* RuntimeContext = DuplicateObject<UInputMappingContext>(SourceContext, this);
	ApplySavedKeybinds(RuntimeContext);
	RuntimeContexts.Add(SourceContext, RuntimeContext);
	return RuntimeContext;
}

void UPotionPanicKeybindSubsystem::ApplySavedKeybinds(UInputMappingContext* RuntimeContext) const
{
	const UPotionPanicUserSettings* Settings = UPotionPanicUserSettings::GetPotionPanicUserSettings();
	if (!Settings || !RuntimeContext)
	{
		return;
	}

	const int32 PlayerIdx = ResolvePlayerIndex();

	TMap<FName, int32> KeyboardIndexCounter;
	TMap<FName, int32> GamepadIndexCounter;

	const int32 NumMappings = RuntimeContext->GetMappings().Num();
	for (int32 i = 0; i < NumMappings; ++i)
	{
		FEnhancedActionKeyMapping& Mapping = RuntimeContext->GetMapping(i);
		if (!Mapping.Action)
		{
			continue;
		}

		const FName ActionName = Mapping.Action->GetFName();
		const bool bGamepad = Mapping.Key.IsGamepadKey();
		int32& Counter = bGamepad ? GamepadIndexCounter.FindOrAdd(ActionName, 0)
		                          : KeyboardIndexCounter.FindOrAdd(ActionName, 0);
		const int32 MappingIndex = Counter++;

		for (const FSavedKeybind& Saved : Settings->SavedKeybinds)
		{
			if (Saved.PlayerIndex != PlayerIdx ||
			    Saved.InputActionName != ActionName ||
			    Saved.MappingIndex != MappingIndex)
			{
				continue;
			}

			const FKey NewKey = bGamepad ? Saved.GamepadKey : Saved.KeyboardKey;
			if (NewKey.IsValid())
			{
				Mapping.Key = NewKey;
			}
			break;
		}
	}
}

int32 UPotionPanicKeybindSubsystem::ResolvePlayerIndex() const
{

	const ULocalPlayer* LocalPlayer = GetLocalPlayer();
	return LocalPlayer ? LocalPlayer->GetControllerId() : 0;
}
