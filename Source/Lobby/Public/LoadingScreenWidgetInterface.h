#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LoadingScreenWidgetInterface.generated.h"

class UTexture2D;

UINTERFACE(MinimalAPI)
class ULoadingScreenWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for the loading screen widget, allowing the loading screen subsystem to configure it
 * without creating a hard dependency on the concrete (Blueprint) widget class.
 */
class LOBBY_API ILoadingScreenWidgetInterface
{
	GENERATED_BODY()

public:

	/** Called right after the widget is created, to set the background image pulled from the level DataTable */
	UFUNCTION(BlueprintNativeEvent, Category = "Loading Screen")
	void SetBackgroundImage(UTexture2D* BackgroundImage);
};
