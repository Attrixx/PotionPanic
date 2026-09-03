// Fill out your copyright notice in the Description page of Project Settings.

#include "LevelSelectorActor.h"
#include "AlchemistBase.h"
#include "LevelProgressionSubsystem.h"
#include "LevelSelectorUIInterface.h"
#include "LoadingScreenSubsystem.h"
#include "Net/UnrealNetwork.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/WidgetComponent.h"
#include "NiagaraComponent.h"


ALevelSelectorActor::ALevelSelectorActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(SceneRoot);

	DoorStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorStaticMeshComponent"));
	DoorStaticMeshComponent->SetupAttachment(RootComponent);

	Torch1NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Torch1NiagaraComponent"));
	Torch1NiagaraComponent->SetupAttachment(RootComponent);

	Torch2NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Torch2NiagaraComponent"));
	Torch2NiagaraComponent->SetupAttachment(RootComponent);

	Torch1PointLightComponent = CreateDefaultSubobject<UPointLightComponent>(TEXT("Torch1PointLightComponent"));
	Torch1PointLightComponent->SetupAttachment(Torch1NiagaraComponent);

	Torch2PointLightComponent = CreateDefaultSubobject<UPointLightComponent>(TEXT("Torch2PointLightComponent"));
	Torch2PointLightComponent->SetupAttachment(Torch2NiagaraComponent);

	LevelIndexWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("LevelIndexWidgetComponent"));
	LevelIndexWidgetComponent->SetupAttachment(DoorStaticMeshComponent);

	OpenDoorTriggerBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("OpenDoorTriggerBoxComponent"));
	OpenDoorTriggerBoxComponent->SetupAttachment(RootComponent);

	LevelLoadTriggerBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("LevelLoadTriggerBoxComponent"));
	LevelLoadTriggerBoxComponent->SetupAttachment(RootComponent);

	DoorOpenAngle = 0.f;
	PlayersInDoorZoneCount = 0;
	bIsReplicatedLocked = true;
}

void ALevelSelectorActor::UnlockLevel()
{
	if (!HasAuthority()) return;

	bool bPreviouslyLocked = LevelData.bIsLocked;
	UpdateLevelData();
	
	if (bPreviouslyLocked != LevelData.bIsLocked)
	{
		bIsReplicatedLocked = LevelData.bIsLocked;
		UpdateVisuals();
		
		Multicast_PlayUnlockEffects();
	}
}

void ALevelSelectorActor::Multicast_PlayUnlockEffects_Implementation()
{	
	if (LevelIndexWidgetComponent)
	{
		if (UUserWidget* Widget = LevelIndexWidgetComponent->GetUserWidgetObject())
		{
			if (Widget->Implements<ULevelSelectorUIInterface>())
			{
				ILevelSelectorUIInterface::Execute_OnLevelUnlocked(Widget);
			}
		}
	}
}

void ALevelSelectorActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALevelSelectorActor, bIsReplicatedLocked);
}

void ALevelSelectorActor::OnRep_IsReplicatedLocked()
{
	LevelData.bIsLocked = bIsReplicatedLocked;
	UpdateVisuals();
}

void ALevelSelectorActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	SetDoorAngle(DoorOpenAngle);
}

void ALevelSelectorActor::UpdateVisuals()
{
	if (Torch1NiagaraComponent) Torch1NiagaraComponent->SetVisibility(!IsLocked());
	if (Torch2NiagaraComponent) Torch2NiagaraComponent->SetVisibility(!IsLocked());
	if (Torch1PointLightComponent) Torch1PointLightComponent->SetVisibility(!IsLocked());
	if (Torch2PointLightComponent) Torch2PointLightComponent->SetVisibility(!IsLocked());
	if (LevelIndexWidgetComponent)
	{
		LevelIndexWidgetComponent->SetVisibility(!IsLocked());
		if (UUserWidget* Widget = LevelIndexWidgetComponent->GetUserWidgetObject())
		{
			if (Widget->Implements<ULevelSelectorUIInterface>())
			{
				ILevelSelectorUIInterface::Execute_SetLevelNumber(Widget, LevelData.LevelNumber);
			}
		}
	}
}

void ALevelSelectorActor::SetDoorAngle(float NewAngle)
{
	DoorOpenAngle = FMath::Clamp(NewAngle, MinDoorAngle, MaxDoorAngle);
	if (DoorStaticMeshComponent)
	{
		DoorStaticMeshComponent->SetRelativeRotation(FRotator(0.f, DoorOpenAngle, 0.f));
	}
}

void ALevelSelectorActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (OpenDoorTriggerBoxComponent)
	{
		OpenDoorTriggerBoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ALevelSelectorActor::OnOpenDoorTriggerOverlap);
		OpenDoorTriggerBoxComponent->OnComponentEndOverlap.AddDynamic(this, &ALevelSelectorActor::OnOpenDoorTriggerEndOverlap);
	}
	
	if (LevelLoadTriggerBoxComponent)
	{
		LevelLoadTriggerBoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ALevelSelectorActor::OnLevelLoadTriggerOverlap);
	}

	UpdateLevelData();
	
	if (HasAuthority())
	{
		bIsReplicatedLocked = LevelData.bIsLocked;
	}
	else
	{
		LevelData.bIsLocked = bIsReplicatedLocked;
	}
	
	UpdateVisuals();
	
	// Reset door to closed state for the game
	SetDoorAngle(0.f);
}

void ALevelSelectorActor::OnOpenDoorTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IsLocked())
	{
		return;
	}

	if (OtherActor && OtherActor->IsA(AAlchemistBase::StaticClass()))
	{
		PlayersInDoorZoneCount++;
		
		if (PlayersInDoorZoneCount == 1)
		{
			OnDoorZoneOccupancyChanged.Broadcast(LevelData, true);
			OnOpenDoor();
		}
	}
}

void ALevelSelectorActor::OnOpenDoorTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (IsLocked())
	{
		return;
	}

	if (OtherActor && OtherActor->IsA(AAlchemistBase::StaticClass()))
	{
		PlayersInDoorZoneCount--;
		
		if (PlayersInDoorZoneCount <= 0)
		{
			PlayersInDoorZoneCount = 0;
			OnDoorZoneOccupancyChanged.Broadcast(LevelData, false);
			OnCloseDoor();
		}
	}
}

void ALevelSelectorActor::OnLevelLoadTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || IsLocked() || !OtherActor || LevelData.Level.IsNull())
	{
		return;
	}
	if (!OtherActor->IsA(AAlchemistBase::StaticClass()))
	{
		return;
	}
	if (UWorld* World = GetWorld())
	{
		Multicast_ShowLoadingScreen();
		World->ServerTravel(LevelData.Level.ToSoftObjectPath().GetLongPackageName());
	}
}

void ALevelSelectorActor::Multicast_ShowLoadingScreen_Implementation()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (ULoadingScreenSubsystem* LoadingScreen = GameInstance->GetSubsystem<ULoadingScreenSubsystem>())
		{
			LoadingScreen->ShowLoadingScreen(LoadingScreenWidgetClass, GetLevelLoadingTexture());
		}
	}
}

UTexture2D* ALevelSelectorActor::GetLevelLoadingTexture() const
{
	if (!LevelDataTable || LevelID.IsNone())
	{
		return nullptr;
	}

	static const FString ContextString(TEXT("ALevelSelectorActor::GetLevelLoadingTexture"));
	if (const FLevelStaticData* StaticData = LevelDataTable->FindRow<FLevelStaticData>(LevelID, ContextString))
	{
		return StaticData->LevelLoadingTexture.LoadSynchronous();
	}
	return nullptr;
}

void ALevelSelectorActor::UpdateLevelData()
{
	if (ULevelProgressionSubsystem* Subsystem = GetGameInstance()->GetSubsystem<ULevelProgressionSubsystem>())
	{
		LevelData = Subsystem->GetLevelData(LevelID, LevelDataTable);
	}
}
