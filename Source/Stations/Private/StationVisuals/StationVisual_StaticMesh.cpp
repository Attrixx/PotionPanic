// Fill out your copyright notice in the Description page of Project Settings.

#include "StationVisuals/StationVisual_StaticMesh.h"
#include "HolderComponent.h"
#include "StationActor.h"

namespace
{
FName ComponentTag = "StationVisual_StaticMesh";
}

void UStationVisual_StaticMesh::Apply(AStationActor* Target)
{
	if (!Target || !ComponentTemplate)
		return;

	auto* SMC = NewObject<UStaticMeshComponent>(Target, NAME_None, RF_NoFlags, ComponentTemplate);
	if (!SMC)
		return;

	SMC->SetupAttachment(Target->GetRootComponent());
	SMC->ComponentTags.Add(ComponentTag);
	SMC->RegisterComponent();

	if (auto* Holder = Target->GetItemHolder())
	{
		Holder->AttachToComponent(SMC, FAttachmentTransformRules::SnapToTargetNotIncludingScale, HolderSocket);
	}
}

void UStationVisual_StaticMesh::Teardown(AStationActor* Target)
{
	if (!Target)
		return;

	if (auto* Holder = Target->GetItemHolder())
	{
		Holder->AttachToComponent(Target->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
	}

	TArray Comps = Target->GetComponentsByTag(UStaticMeshComponent::StaticClass(), ComponentTag);
	for (auto* Comp : Comps)
	{
		Comp->DestroyComponent();
	}
}
