#include "PotionPanicInputDetectionSubsystem.h"
#include "CommonInputSubsystem.h"
#include "Engine/LocalPlayer.h"

#include "hidapi.h"

namespace
{

	constexpr uint16 VID_Microsoft = 0x045E;
	constexpr uint16 VID_Sony      = 0x054C;

	FName ResolveGamepadName(uint16 Vid, uint16 )
	{
		switch (Vid)
		{
		case VID_Microsoft: return TEXT("XBox");
		case VID_Sony:      return TEXT("PlayStation");
		default:            return NAME_None;
		}
	}
}

void UPotionPanicInputDetectionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency(UCommonInputSubsystem::StaticClass());
	Super::Initialize(Collection);

	IPlatformInputDeviceMapper& Mapper = IPlatformInputDeviceMapper::Get();
	ConnectionDelegateHandle = Mapper.GetOnInputDeviceConnectionChange().AddUObject(this, &ThisClass::OnInputDeviceConnectionChange);

	RefreshGamepadType();
}

void UPotionPanicInputDetectionSubsystem::Deinitialize()
{
	if (ConnectionDelegateHandle.IsValid())
	{
		IPlatformInputDeviceMapper::Get().GetOnInputDeviceConnectionChange().Remove(ConnectionDelegateHandle);
		ConnectionDelegateHandle.Reset();
	}

	hid_exit();

	Super::Deinitialize();
}

void UPotionPanicInputDetectionSubsystem::OnInputDeviceConnectionChange(EInputDeviceConnectionState NewState, FPlatformUserId, FInputDeviceId)
{
	if (NewState == EInputDeviceConnectionState::Connected)
	{
		RefreshGamepadType();
	}
}

void UPotionPanicInputDetectionSubsystem::RefreshGamepadType() const
{
	const FName Detected = DetectConnectedGamepadName();
	if (Detected.IsNone())
	{
		return;
	}

	if (UCommonInputSubsystem* CommonInput = UCommonInputSubsystem::Get(GetLocalPlayer()))
	{
		CommonInput->SetGamepadInputType(Detected);
		UE_LOG(LogTemp, Verbose, TEXT("[InputDetection] Pushed %s to CommonInput"), *Detected.ToString());
	}
}

FName UPotionPanicInputDetectionSubsystem::DetectConnectedGamepadName()
{
	if (hid_init() != 0)
	{
		return NAME_None;
	}

	FName Result = NAME_None;

	if (hid_device_info* Devices = hid_enumerate(0x0, 0x0))
	{
		for (const hid_device_info* Device = Devices; Device != nullptr; Device = Device->next)
		{
			const uint16 Vid = static_cast<uint16>(Device->vendor_id);
			const uint16 Pid = static_cast<uint16>(Device->product_id);

			UE_LOG(LogTemp, Verbose, TEXT("[InputDetection] HID device VID=0x%04X PID=0x%04X"), Vid, Pid);

			const FName Resolved = ResolveGamepadName(Vid, Pid);
			if (!Resolved.IsNone())
			{
				Result = Resolved;
				break;
			}
		}

		hid_free_enumeration(Devices);
	}

	return Result;
}
