#pragma once

#include "CoreMinimal.h"
#include "KeybindTypes.h"
#include "KeybindManager.generated.h"

class UInputMappingContext;
class UEnhancedInputLocalPlayerSubsystem;

UCLASS(BlueprintType)
class USERINTERFACES_API UKeybindManager : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Keybind")
	void InitializeFromIMC(UInputMappingContext* IMC);

	UFUNCTION(BlueprintCallable, Category = "Keybind")
	bool RebindKey(FName ActionName, int32 MappingIndex, FKey NewKey, bool bIsGamepad);

	UFUNCTION(BlueprintCallable, Category = "Keybind")
	bool HasDuplicateKey(FKey Key, bool bIsGamepad, FName ExcludeAction, int32 ExcludeIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Keybind")
	FKeybindEntry GetDuplicateEntry(FKey Key, bool bIsGamepad, FName ExcludeAction, int32 ExcludeIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Keybind")
	void ResetBinding(FName ActionName, int32 MappingIndex);

	UFUNCTION(BlueprintCallable, Category = "Keybind")
	void ResetAllBindings();

	UFUNCTION(BlueprintCallable, Category = "Keybind")
	void ApplyToIMC(UInputMappingContext* IMC, UEnhancedInputLocalPlayerSubsystem* Subsystem);

	UFUNCTION(BlueprintCallable, Category = "Keybind")
	void Save();

	UFUNCTION(BlueprintCallable, Category = "Keybind")
	void Load();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Keybind")
	TArray<FKeybindEntry> GetBindingsForDevice(bool bGamepad) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Keybind")
	const TArray<FKeybindEntry>& GetAllBindings() const { return Bindings; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Keybind")
	bool IsDirty() const { return bDirty; }

	TMap<FName, FText> DisplayNameOverrides;

	UFUNCTION(BlueprintCallable, Category = "Keybind")
	void SetPlayerIndex(int32 InPlayerIndex) { PlayerIndex = InPlayerIndex; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Keybind")
	int32 GetPlayerIndex() const { return PlayerIndex; }

private:

	FKeybindEntry* FindBinding(FName ActionName, int32 MappingIndex);
	const FKeybindEntry* FindBinding(FName ActionName, int32 MappingIndex) const;

	TArray<FKeybindEntry> Bindings;
	int32 PlayerIndex = 0;
	bool bDirty = false;
};
