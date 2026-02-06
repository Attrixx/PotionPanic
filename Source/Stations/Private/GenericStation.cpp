#include "GenericStation.h"
#include "StationDataAsset.h"
#include "Components/StaticMeshComponent.h"
#include "Logging/StructuredLog.h"

DEFINE_LOG_CATEGORY_STATIC(LogGenericStation, Log, All);

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

	// Ensure data is applied at runtime too
	ApplyStationData();
}

void AGenericStation::ApplyStationData()
{
	if (!StationData)
	{
		return;
	}

	// Apply Mesh
	if (!StationData->StationMesh.IsNull())
	{
		UStaticMesh* Mesh = StationData->StationMesh.LoadSynchronous();
		if (Mesh && StationMesh)
		{
			StationMesh->SetStaticMesh(Mesh);
		}
	}

	// Apply Config
	InteractionDistance = StationData->InteractionDistance;

	// Copy Arrays
	// Note: We are copying data from Asset to Instance.
	// This allows instance-specific overrides if needed later, but primarily drives logic from Data.
	Activities = StationData->SupportedActivities;
	PossibleInstructions = StationData->Recipes;
}
