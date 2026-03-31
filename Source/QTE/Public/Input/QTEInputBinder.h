#pragma once

#include "CoreMinimal.h"

class UEnhancedInputComponent;
class UInputAction;
class UQTEComponent;

class QTE_API FQTEInputBinder
{
public:
	FQTEInputBinder();
	explicit FQTEInputBinder(UQTEComponent& InOwnerComponent);
	~FQTEInputBinder();

	void InitializeOwner(UQTEComponent& InOwnerComponent);
	void Initialize(UEnhancedInputComponent* InEnhancedInputComponent);
	void BindForDefinition();
	void Unbind();
	void AddMappingContext();
	void RemoveMappingContext();
	bool HasEnhancedInputComponent() const;

private:
	UQTEComponent& GetOwnerComponent() const;

	struct FBoundInputHandles
	{
		uint32 StartedHandle = 0;
		uint32 TriggeredHandle = 0;
		uint32 CompletedHandle = 0;
		uint32 CanceledHandle = 0;
	};

	UQTEComponent* OwnerComponent = nullptr;
	TWeakObjectPtr<UEnhancedInputComponent> EnhancedInputComponent;
	TMap<TObjectPtr<UInputAction>, FBoundInputHandles> BoundInputHandles;
	bool bAddedMappingContext = false;
};
