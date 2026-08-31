#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "PotionPanicInputDetectionSubsystem.generated.h"

UCLASS()
class USERINTERFACES_API UPotionPanicInputDetectionSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:

	void OnInputDeviceConnectionChange(EInputDeviceConnectionState NewState, FPlatformUserId UserId, FInputDeviceId DeviceId);
	void RefreshGamepadType() const;

	static FName DetectConnectedGamepadName();

	FDelegateHandle ConnectionDelegateHandle;
};
