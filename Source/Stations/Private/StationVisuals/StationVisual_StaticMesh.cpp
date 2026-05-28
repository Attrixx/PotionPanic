// Fill out your copyright notice in the Description page of Project Settings.

#include "StationVisuals/StationVisual_StaticMesh.h"
#include "HolderComponent.h"
#include "StationActor.h"

void UStationVisual_StaticMesh::Apply(AStationActor* Target)
{
	if (!Target || !ComponentTemplate)
		return;
	
	if (auto* SMC = NewObject<UStaticMeshComponent>(Target, NAME_None, RF_NoFlags, ComponentTemplate))
	{
		SMC->SetupAttachment(Target->GetRootComponent());
		SMC->RegisterComponent();

		if (auto* Holder = Target->GetItemHolder())
		{
			Holder->AttachToComponent(SMC, FAttachmentTransformRules::SnapToTargetNotIncludingScale, HolderSocket);
		}
	}
}

void UStationVisual_StaticMesh::Teardown(AStationActor* Target)
{
	if (auto* SMC = Target->GetComponentByClass<UStaticMeshComponent>())
	{
		SMC->DestroyComponent(true);
	}
}
