#include "KeybindManager.h"
#include "PotionPanicUserSettings.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "EnhancedInputSubsystems.h"

void UKeybindManager::InitializeFromIMC(UInputMappingContext* IMC)
{
	if (!IMC) return;

	Bindings.Empty();

	TMap<FName, int32> KeyboardIndexCounter;
	TMap<FName, int32> GamepadIndexCounter;

	const int32 NumMappings = IMC->GetMappings().Num();

	for (int32 i = 0; i < NumMappings; ++i)
	{
		const FEnhancedActionKeyMapping& Mapping = IMC->GetMapping(i);
		if (!Mapping.Action) continue;

		const FName ActionName = Mapping.Action->GetFName();
		const FKey& Key = Mapping.Key;

		if (Key.IsGamepadKey()) continue;

		int32& Counter = KeyboardIndexCounter.FindOrAdd(ActionName, 0);

		FKeybindEntry NewEntry;
		NewEntry.InputActionName = ActionName;
		NewEntry.MappingIndex = Counter;
		NewEntry.KeyboardKey = Key;
		NewEntry.DefaultKeyboardKey = Key;

		FName UniqueId = FName(*FString::Printf(TEXT("%s_%d"), *ActionName.ToString(), Counter));
		if (const FText* Override = DisplayNameOverrides.Find(UniqueId))
		{
			NewEntry.DisplayName = *Override;
		}
		else
		{
			NewEntry.DisplayName = FText::FromName(ActionName);
		}

		Bindings.Add(NewEntry);
		Counter++;
	}

	for (int32 i = 0; i < NumMappings; ++i)
	{
		const FEnhancedActionKeyMapping& Mapping = IMC->GetMapping(i);
		if (!Mapping.Action) continue;

		const FName ActionName = Mapping.Action->GetFName();
		const FKey& Key = Mapping.Key;

		if (!Key.IsGamepadKey()) continue;

		int32& Counter = GamepadIndexCounter.FindOrAdd(ActionName, 0);

		FKeybindEntry* Entry = FindBinding(ActionName, Counter);
		if (Entry)
		{
			Entry->GamepadKey = Key;
			Entry->DefaultGamepadKey = Key;
		}

		Counter++;
	}

	bDirty = false;
}

bool UKeybindManager::RebindKey(FName ActionName, int32 MappingIndex, FKey NewKey, bool bIsGamepad)
{
	FKeybindEntry* Entry = FindBinding(ActionName, MappingIndex);
	if (!Entry) return false;

	if (bIsGamepad)
	{
		Entry->GamepadKey = NewKey;
	}
	else
	{
		Entry->KeyboardKey = NewKey;
	}

	bDirty = true;
	return true;
}

bool UKeybindManager::HasDuplicateKey(FKey Key, bool bIsGamepad, FName ExcludeAction, int32 ExcludeIndex) const
{
	for (const FKeybindEntry& Entry : Bindings)
	{
		if (Entry.InputActionName == ExcludeAction && Entry.MappingIndex == ExcludeIndex)
			continue;

		const FKey& BoundKey = bIsGamepad ? Entry.GamepadKey : Entry.KeyboardKey;
		if (BoundKey == Key)
			return true;
	}
	return false;
}

FKeybindEntry UKeybindManager::GetDuplicateEntry(FKey Key, bool bIsGamepad, FName ExcludeAction, int32 ExcludeIndex) const
{
	for (const FKeybindEntry& Entry : Bindings)
	{
		if (Entry.InputActionName == ExcludeAction && Entry.MappingIndex == ExcludeIndex)
			continue;

		const FKey& BoundKey = bIsGamepad ? Entry.GamepadKey : Entry.KeyboardKey;
		if (BoundKey == Key)
			return Entry;
	}
	return FKeybindEntry();
}

void UKeybindManager::ResetBinding(FName ActionName, int32 MappingIndex)
{
	FKeybindEntry* Entry = FindBinding(ActionName, MappingIndex);
	if (!Entry) return;

	Entry->KeyboardKey = Entry->DefaultKeyboardKey;
	Entry->GamepadKey = Entry->DefaultGamepadKey;
	bDirty = true;
}

void UKeybindManager::ResetAllBindings()
{
	for (FKeybindEntry& Entry : Bindings)
	{
		Entry.KeyboardKey = Entry.DefaultKeyboardKey;
		Entry.GamepadKey = Entry.DefaultGamepadKey;
	}
	bDirty = true;
}

void UKeybindManager::ApplyToIMC(UInputMappingContext* IMC, UEnhancedInputLocalPlayerSubsystem* Subsystem)
{
	if (!IMC || !Subsystem) return;

	Subsystem->RemoveMappingContext(IMC);

	TMap<FName, int32> KeyboardIndexCounter;
	TMap<FName, int32> GamepadIndexCounter;

	const int32 NumMappings = IMC->GetMappings().Num();
	for (int32 i = 0; i < NumMappings; ++i)
	{
		FEnhancedActionKeyMapping& Mapping = IMC->GetMapping(i);
		if (!Mapping.Action) continue;

		const FName ActionName = Mapping.Action->GetFName();
		const bool bGamepad = Mapping.Key.IsGamepadKey();

		if (bGamepad)
		{
			int32& Counter = GamepadIndexCounter.FindOrAdd(ActionName, 0);
			const FKeybindEntry* Entry = FindBinding(ActionName, Counter);
			if (Entry && Entry->GamepadKey.IsValid())
			{
				Mapping.Key = Entry->GamepadKey;
			}
			Counter++;
		}
		else
		{
			int32& Counter = KeyboardIndexCounter.FindOrAdd(ActionName, 0);
			const FKeybindEntry* Entry = FindBinding(ActionName, Counter);
			if (Entry && Entry->KeyboardKey.IsValid())
			{
				Mapping.Key = Entry->KeyboardKey;
			}
			Counter++;
		}
	}

	Subsystem->AddMappingContext(IMC, 0);
	bDirty = false;
}

void UKeybindManager::Save()
{
	UPotionPanicUserSettings* Settings = UPotionPanicUserSettings::GetPotionPanicUserSettings();
	if (!Settings) return;

	Settings->SavedKeybinds.RemoveAll([this](const FSavedKeybind& Saved)
	{
		return Saved.PlayerIndex == PlayerIndex;
	});

	for (const FKeybindEntry& Entry : Bindings)
	{
		bool bChanged = (Entry.KeyboardKey != Entry.DefaultKeyboardKey) ||
		                (Entry.GamepadKey != Entry.DefaultGamepadKey);
		if (bChanged)
		{
			FSavedKeybind Saved;
			Saved.PlayerIndex     = PlayerIndex;
			Saved.InputActionName = Entry.InputActionName;
			Saved.MappingIndex    = Entry.MappingIndex;
			Saved.KeyboardKey     = Entry.KeyboardKey;
			Saved.GamepadKey      = Entry.GamepadKey;
			Settings->SavedKeybinds.Add(Saved);
		}
	}

	Settings->SaveSettings();
}

void UKeybindManager::Load()
{
	UPotionPanicUserSettings* Settings = UPotionPanicUserSettings::GetPotionPanicUserSettings();
	if (!Settings) return;

	for (const FSavedKeybind& Saved : Settings->SavedKeybinds)
	{
		if (Saved.PlayerIndex != PlayerIndex) continue;

		FKeybindEntry* Entry = FindBinding(Saved.InputActionName, Saved.MappingIndex);
		if (Entry)
		{
			Entry->KeyboardKey = Saved.KeyboardKey;
			Entry->GamepadKey  = Saved.GamepadKey;
		}
	}
}

TArray<FKeybindEntry> UKeybindManager::GetBindingsForDevice(bool bGamepad) const
{
	TArray<FKeybindEntry> Result;
	for (const FKeybindEntry& Entry : Bindings)
	{
		const FKey& Key = bGamepad ? Entry.GamepadKey : Entry.KeyboardKey;
		if (Key.IsValid())
		{
			Result.Add(Entry);
		}
	}
	return Result;
}

FKeybindEntry* UKeybindManager::FindBinding(FName ActionName, int32 MappingIndex)
{
	for (FKeybindEntry& Entry : Bindings)
	{
		if (Entry.InputActionName == ActionName && Entry.MappingIndex == MappingIndex)
			return &Entry;
	}
	return nullptr;
}

const FKeybindEntry* UKeybindManager::FindBinding(FName ActionName, int32 MappingIndex) const
{
	for (const FKeybindEntry& Entry : Bindings)
	{
		if (Entry.InputActionName == ActionName && Entry.MappingIndex == MappingIndex)
			return &Entry;
	}
	return nullptr;
}
