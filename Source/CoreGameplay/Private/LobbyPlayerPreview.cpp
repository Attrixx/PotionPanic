// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyPlayerPreview.h"

#include "Kismet/KismetMathLibrary.h"

ALobbyPlayerPreview::ALobbyPlayerPreview()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	PlayerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Player Mesh"));
	RootComponent = PlayerMesh;
}

void ALobbyPlayerPreview::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
}

void ALobbyPlayerPreview::BeginPlay()
{
	Super::BeginPlay();
	/*FColor TargetColor(UKismetMathLibrary::RandomIntegerInRange(0, 255), UKismetMathLibrary::RandomIntegerInRange(0, 255), UKismetMathLibrary::RandomIntegerInRange(0, 255));
	SetPlayerColor(TargetColor);*/
}

void ALobbyPlayerPreview::OnRep_PreviewColor()
{
	PlayerMesh->CreateDynamicMaterialInstance(0)->SetVectorParameterValue(FName("Color"), PreviewColor);
}

void ALobbyPlayerPreview::SetPlayerColor_Implementation(FColor Color)
{
	PreviewColor = Color;
	if (HasAuthority())
	{
		OnRep_PreviewColor();
	}
}

