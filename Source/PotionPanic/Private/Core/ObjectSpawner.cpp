#include "Core/ObjectSpawner.h"
#include "Core/SocketComponent.h"
#include "RecipeSystem/StationComponent.h"
#include "Core/SpawnerComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

AObjectSpawner::AObjectSpawner()
{
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	SocketComponent = CreateDefaultSubobject<USocketComponent>(TEXT("SocketComponent"));
	SocketComponent->SetupAttachment(RootComponent);

	StationComponent = CreateDefaultSubobject<UStationComponent>(TEXT("StationComponent"));
	
	SpawnerComponent = CreateDefaultSubobject<USpawnerComponent>(TEXT("SpawnerComponent"));
}

void AObjectSpawner::BeginPlay()
{
	Super::BeginPlay();

	StationComponent->OnEndProcess.AddUObject(SpawnerComponent, &USpawnerComponent::RequestSpawnItem);
}