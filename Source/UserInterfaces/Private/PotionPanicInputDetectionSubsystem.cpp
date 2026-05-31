#include "PotionPanicInputDetectionSubsystem.h"
#include "CommonInputSubsystem.h"
#include "Engine/LocalPlayer.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <Windows.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

namespace
{

	constexpr uint16 VID_Microsoft = 0x045E;
	constexpr uint16 VID_Sony      = 0x054C;

	FName ResolveGamepadName(uint16 Vid, uint16 )
	{
		switch (Vid)
		{
		case VID_Microsoft: return TEXT("XBoxOne");
		case VID_Sony:      return TEXT("PS5");
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
#if PLATFORM_WINDOWS
	UINT NumDevices = 0;
	if (GetRawInputDeviceList(nullptr, &NumDevices, sizeof(RAWINPUTDEVICELIST)) != 0 || NumDevices == 0)
	{
		return NAME_None;
	}

	TArray<RAWINPUTDEVICELIST> Devices;
	Devices.SetNum(NumDevices);
	if (GetRawInputDeviceList(Devices.GetData(), &NumDevices, sizeof(RAWINPUTDEVICELIST)) == static_cast<UINT>(-1))
	{
		return NAME_None;
	}

	for (const RAWINPUTDEVICELIST& Device : Devices)
	{
		if (Device.dwType != RIM_TYPEHID)
		{
			continue;
		}

		RID_DEVICE_INFO Info = {};
		Info.cbSize = sizeof(Info);
		UINT InfoSize = sizeof(Info);
		if (GetRawInputDeviceInfo(Device.hDevice, RIDI_DEVICEINFO, &Info, &InfoSize) <= 0)
		{
			continue;
		}

		const uint16 Vid = static_cast<uint16>(Info.hid.dwVendorId);
		const uint16 Pid = static_cast<uint16>(Info.hid.dwProductId);

		UE_LOG(LogTemp, Verbose, TEXT("[InputDetection] HID device VID=0x%04X PID=0x%04X"), Vid, Pid);

		const FName Resolved = ResolveGamepadName(Vid, Pid);
		if (!Resolved.IsNone())
		{
			return Resolved;
		}
	}
#endif
	return NAME_None;
}
