#include "GenericStation.h"
#include "Components/StaticMeshComponent.h"
#include "Logging/StructuredLog.h"
#include "StationDataAsset.h"

DEFINE_LOG_CATEGORY_STATIC(MS_GenericStation, Log, All);

AGenericStation::AGenericStation()
{
}

void AGenericStation::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyStationData();
}

void AGenericStation::BeginPlay()
{
	Super::BeginPlay();
	ApplyStationData();
}

#if WITH_EDITOR
void AGenericStation::CheckForErrors()
{
	Super::CheckForErrors();

	if (StationData == nullptr)
	{
		UE_LOGFMT(MS_GenericStation, Warning, "GenericStation '{0}' has no StationData asset assigned.", GetName());
		return;
	}

	if (StationData->StationMesh.IsNull())
	{
		UE_LOGFMT(MS_GenericStation, Warning, "GenericStation '{0}' StationData has no StationMesh configured.", GetName());
	}

	if (StationData->Instructions.Num() == 0)
	{
		UE_LOGFMT(MS_GenericStation, Warning, "GenericStation '{0}' StationData has no Instructions configured.", GetName());
	}

	if (StationData->SupportedActivities.Num() == 0)
	{
		UE_LOGFMT(MS_GenericStation, Warning, "GenericStation '{0}' StationData has no SupportedActivities configured.", GetName());
	}
}
#endif

void AGenericStation::ApplyStationData()
{
	if (StationData == nullptr)
	{
		return;
	}

	if (!StationData->StationMesh.IsNull() && StationMesh)
	{
		UStaticMesh* Mesh = StationData->StationMesh.LoadSynchronous();
		if (Mesh)
		{
			StationMesh->SetStaticMesh(Mesh);
		}
	}

	InteractionDistance = StationData->InteractionDistance;
	Activities = StationData->SupportedActivities;
	PossibleInstructions = StationData->Instructions;
}
