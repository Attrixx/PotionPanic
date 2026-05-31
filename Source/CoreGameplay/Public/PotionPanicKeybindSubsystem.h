#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "PotionPanicKeybindSubsystem.generated.h"

class UInputMappingContext;

UCLASS()
class COREGAMEPLAY_API UPotionPanicKeybindSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Keybind")
	UInputMappingContext* GetRuntimeContext(UInputMappingContext* SourceContext);

private:

	void ApplySavedKeybinds(UInputMappingContext* RuntimeContext) const;
	int32 ResolvePlayerIndex() const;

	UPROPERTY()
	TMap<TObjectPtr<UInputMappingContext>, TObjectPtr<UInputMappingContext>> RuntimeContexts;
};
