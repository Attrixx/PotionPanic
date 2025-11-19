#include "Core/CauldronStation.h"
#include "Core/SocketComponent.h"
#include "RecipeSystem/StationComponent.h"
#include "Core/SpawnerComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Core/SocketableComponent.h"
#include "Core/SpawnerComponent.h"

ACauldronStation::ACauldronStation()
{
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	SocketComponent = CreateDefaultSubobject<USocketComponent>(TEXT("SocketComponent"));
	SocketComponent->SetupAttachment(RootComponent);

	SpawnerComponent = CreateDefaultSubobject<USpawnerComponent>(TEXT("SpawnerComponent"));

	StationComponent = CreateDefaultSubobject<UStationComponent>(TEXT("StationComponent"));
}

void ACauldronStation::BeginPlay()
{
	Super::BeginPlay();

	SocketComponent->OnPut.AddUObject(this, &ACauldronStation::Store);
	StationComponent->OnEndProcess.AddUObject(SpawnerComponent, &USpawnerComponent::SpawnItem);
}

void ACauldronStation::Store()
{
	if (!SocketComponent->IsHolding())
		return;

	USocketableComponent* ItemComponent = SocketComponent->Take();
	AActor* Item = ItemComponent->GetOwner();

	StationComponent->Store(Item->GetClass());
	Item->Destroy();
}