// Fill out your copyright notice in the Description page of Project Settings.

#include "LoadingScreenSubsystem.h"
#include "LoadingScreenWidgetInterface.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "GameMapsSettings.h"

void ULoadingScreenSubsystem::Deinitialize()
{
	HideLoadingScreen();
	Super::Deinitialize();
}

void ULoadingScreenSubsystem::ShowLoadingScreen(TSubclassOf<UUserWidget> WidgetClass, UTexture2D* BackgroundImage)
{
	if (LoadingScreenWidget || !WidgetClass)
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	LoadingScreenWidget = CreateWidget<UUserWidget>(GameInstance, WidgetClass);
	if (!LoadingScreenWidget)
	{
		return;
	}

	BackgroundImageRef = BackgroundImage;

	if (LoadingScreenWidget->Implements<ULoadingScreenWidgetInterface>())
	{
		ILoadingScreenWidgetInterface::Execute_SetBackgroundImage(LoadingScreenWidget, BackgroundImage);
	}

	LoadingScreenWidget->AddToViewport(LoadingScreenZOrder);

	// Remove the loading screen once the destination level has finished loading.
	if (!PostLoadMapHandle.IsValid())
	{
		PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ULoadingScreenSubsystem::HandlePostLoadMap);
	}
}

void ULoadingScreenSubsystem::HideLoadingScreen()
{
	if (PostLoadMapHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
		PostLoadMapHandle.Reset();
	}

	if (LoadingScreenWidget)
	{
		LoadingScreenWidget->RemoveFromParent();
		LoadingScreenWidget = nullptr;
	}

	BackgroundImageRef = nullptr;
}

void ULoadingScreenSubsystem::HandlePostLoadMap(UWorld* LoadedWorld)
{
	if (!LoadedWorld)
	{
		return;
	}

	// During seamless travel the engine briefly loads the transition map before the real
	// destination - keep the loading screen up until the destination is actually loaded.
	const FString TransitionMap = GetDefault<UGameMapsSettings>()->TransitionMap.GetLongPackageName();
	if (!TransitionMap.IsEmpty() && LoadedWorld->GetOutermost()->GetName() == TransitionMap)
	{
		return;
	}

	HideLoadingScreen();
}
