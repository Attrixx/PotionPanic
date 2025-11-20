#include "Core/CutStation.h"
#include "Core/SocketComponent.h"
#include "RecipeSystem/StationComponent.h"
#include "Core/SpawnerComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Core/SocketableComponent.h"

ACutStation::ACutStation()
{
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	SocketComponent = CreateDefaultSubobject<USocketComponent>(TEXT("SocketComponent"));
	SocketComponent->SetupAttachment(RootComponent);

	SpawnerComponent = CreateDefaultSubobject<USpawnerComponent>(TEXT("SpawnerComponent"));

	StationComponent = CreateDefaultSubobject<UStationComponent>(TEXT("StationComponent"));
}

void ACutStation::BeginPlay()
{
	Super::BeginPlay();

	StationComponent->OnEndProcess.AddUObject(this, &ACutStation::RemoveOnSocket);
	StationComponent->OnEndProcess.AddUObject(SpawnerComponent, &USpawnerComponent::SpawnItem);
}

void ACutStation::RemoveOnSocket(APawn* InInstigator, TSubclassOf<AActor> Item)
{
	SocketComponent->Take()->GetOwner()->Destroy();
}
