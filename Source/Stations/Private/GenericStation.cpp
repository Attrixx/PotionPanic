#include "GenericStation.h"
#include "Components/StaticMeshComponent.h"
#include "StationDataAsset.h"

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
