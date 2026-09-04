// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemVisualActor.h"

AItemVisualActor::AItemVisualActor()
{
	PrimaryActorTick.bCanEverTick = false;
	SetActorEnableCollision(false);
	bReplicates = false;
}
