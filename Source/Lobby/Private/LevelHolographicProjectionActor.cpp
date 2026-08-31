// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelHolographicProjectionActor.h"
#include "Net/UnrealNetwork.h"
#include "Components/WidgetComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "LobbyGameState.h"

ALevelHolographicProjectionActor::ALevelHolographicProjectionActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRootComponent"));
	SetRootComponent(SceneRootComponent);

	HolographicProjectionWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HolographicProjectionWidgetComponent"));
	HolographicProjectionWidgetComponent->SetupAttachment(RootComponent);

	bReplicates = true;
	bIsShowing = false;
}

void ALevelHolographicProjectionActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (ALobbyGameState* LobbyGameState = GetWorld()->GetGameState<ALobbyGameState>())
	{
		LobbyGameState->RegisterLevelHolographicProjection(this);
	}
}

void ALevelHolographicProjectionActor::SetProgress(float InProgress)
{
	if (HolographicProjectionMaterialParameterCollection)
	{
		UMaterialParameterCollectionInstance* MPCInstance = GetWorld()->GetParameterCollectionInstance(HolographicProjectionMaterialParameterCollection);
		MPCInstance->SetScalarParameterValue(OpacityParameterName, InProgress);
		MPCInstance->SetScalarParameterValue(RaysLengthParameterName, FMath::Lerp(0.f, MaxRayLength, InProgress));
	}

	if (HolographicProjectionWidgetComponent)
	{
		HolographicProjectionWidgetComponent->GetWidget()->SetRenderOpacity(InProgress);
	}
}

void ALevelHolographicProjectionActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALevelHolographicProjectionActor, bIsShowing);
	DOREPLIFETIME(ALevelHolographicProjectionActor, CurrentLevelData);
}

void ALevelHolographicProjectionActor::SetIsShowing(bool bNewShowing)
{
	if (HasAuthority() && bIsShowing != bNewShowing)
	{
		bIsShowing = bNewShowing;
		OnRep_IsShowing();
	}
}

void ALevelHolographicProjectionActor::OnRep_IsShowing()
{
	if (bIsShowing)
	{
		Show();
	}
	else
	{
		Hide();
	}
}

void ALevelHolographicProjectionActor::SetLevelData(const FLevelData& NewData)
{
	if (HasAuthority())
	{
		CurrentLevelData = NewData;
		OnRep_LevelData();
	}
}

void ALevelHolographicProjectionActor::OnRep_LevelData()
{
	OnLevelDataChanged.Broadcast(CurrentLevelData);
}
