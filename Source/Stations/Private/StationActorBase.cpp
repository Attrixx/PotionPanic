// Fill out your copyright notice in the Description page of Project Settings.

#include "StationActorBase.h"

AStationActorBase::AStationActorBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AStationActorBase::Interact(APlayerController& InInstigator)
{
	// TODO
}

void AStationActorBase::BeginPlay()
{
	Super::BeginPlay();
}

