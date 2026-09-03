// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LoadingScreenSubsystem.generated.h"

class UUserWidget;
class UTexture2D;

/**
 * Displays a full screen loading widget while a level is being loaded (e.g. after a server travel
 * triggered from a Level Selector door).
 *
 * The widget is owned by the game instance so it survives the map transition, and is automatically
 * removed once the destination level has finished loading.
 */
UCLASS()
class LOBBY_API ULoadingScreenSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	virtual void Deinitialize() override;

	/**
	 * Creates and shows the loading screen widget.
	 * @param WidgetClass		The widget class to instantiate (should implement ILoadingScreenWidgetInterface).
	 * @param BackgroundImage	Background image for the loading screen, pulled from the level DataTable.
	 */
	void ShowLoadingScreen(TSubclassOf<UUserWidget> WidgetClass, UTexture2D* BackgroundImage);

	/** Removes the loading screen widget if it is currently shown. */
	void HideLoadingScreen();

private:

	void HandlePostLoadMap(UWorld* LoadedWorld);

	UPROPERTY()
	TObjectPtr<UUserWidget> LoadingScreenWidget;

	/** Keeps the background texture alive across the map transition. */
	UPROPERTY()
	TObjectPtr<UTexture2D> BackgroundImageRef;

	FDelegateHandle PostLoadMapHandle;

	static constexpr int32 LoadingScreenZOrder = 10000;
};
