// Fill out your copyright notice in the Description page of Project Settings.

#include "AlchemistHUDWidget.h"

UAlchemistHUDWidget::UAlchemistHUDWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// The one config the screens above it must be able to fall back to. Locked here rather than
	// left to the Blueprint's defaults: a HUD set to Menu would freeze the game on construction.
	InputMode = ECommonInputMode::Game;
	MouseCaptureMode = EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown;

	// Active from the moment it is built, so it is there to fall back to (inherited, restated).
	bAutoActivate = true;

	// The HUD is not a menu: Escape must reach the pause menu binding rather than end here.
	bIsBackHandler = false;
}
