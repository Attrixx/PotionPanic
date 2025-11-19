#include "Core/DeliveryStation.h"
#include "Core/SocketComponent.h"
#include "Core/DeliveryComponent.h"
#include "RecipeSystem/StationComponent.h"
#include "Core/SpawnerComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

ADeliveryStation::ADeliveryStation()
{
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	SocketComponent = CreateDefaultSubobject<USocketComponent>(TEXT("SocketComponent"));
	SocketComponent->SetupAttachment(RootComponent);

	StationComponent = CreateDefaultSubobject<UStationComponent>(TEXT("StationComponent"));

	DeliveryComponent = CreateDefaultSubobject<UDeliveryComponent>(TEXT("DeliveryComponent"));
}

void ADeliveryStation::BeginPlay()
{
	Super::BeginPlay();

	StationComponent->OnBeginProcess.AddUObject(DeliveryComponent, &UDeliveryComponent::Deliver);
}