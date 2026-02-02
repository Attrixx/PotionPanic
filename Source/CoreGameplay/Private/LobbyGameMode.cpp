// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"

#include "LobbySpawnPoint.h"       
#include "LobbyPlayerPreview.h"    
#include "LobbyPlayerController.h" 
#include "Kismet/GameplayStatics.h" 
ALobbyGameMode::ALobbyGameMode()
{
	PlayerControllerClass = ALobbyPlayerController::StaticClass();
}

void ALobbyGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	PlayerCount++;
	UE_LOG(LogTemp, Log, TEXT("Joueur connecte"));

	Super::PostLogin(NewPlayer);

	TArray<AActor*> FoundPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALobbySpawnPoint::StaticClass(), FoundPoints);

	ALobbySpawnPoint* ChosenPoint = nullptr;

	for (AActor* Actor : FoundPoints)
	{
		ALobbySpawnPoint* TestPoint = Cast<ALobbySpawnPoint>(Actor);
		if (TestPoint && !TestPoint->bIsOccupied)
		{
			ChosenPoint = TestPoint;
			break;
		}
	}
	if (ChosenPoint)
	{
		ChosenPoint->bIsOccupied = true; 
		FActorSpawnParameters SpawnParams; 
		SpawnParams.Owner = NewPlayer; 
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ALobbyPlayerPreview* NewPreview = GetWorld()->SpawnActor<ALobbyPlayerPreview>(
			ALobbyPlayerPreview::StaticClass(),
			ChosenPoint->GetActorLocation(),
			ChosenPoint->GetActorRotation(),
			SpawnParams
			);

		if (ALobbyPlayerController* MyPc = Cast<ALobbyPlayerController>(NewPlayer))
		{
			MyPc->MyPreviewActor = NewPreview;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Pas de SpawnPoint libre trouvé !"));
	}

}

	

void ALobbyGameMode::Logout(AController* Exiting)
{
	if (ALobbyPlayerController* LeavingPC = Cast<ALobbyPlayerController>(Exiting))
	{
		TArray<AActor*> FoundPoints; 
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALobbySpawnPoint::StaticClass(), FoundPoints);

		//SAVE SPAWNPOINT IN PC 
		for (AActor* Actor : FoundPoints)
		{
			ALobbySpawnPoint* Point = Cast<ALobbySpawnPoint>(Actor); 
			if (Point && FVector::DistSquared(Point->GetActorLocation(), LeavingPC->MyPreviewActor->GetActorLocation()) < 2500.0f)
			{

				Point->bIsOccupied = false;
				break;
			}
		}
		LeavingPC->MyPreviewActor->Destroy();
	}


	PlayerCount--;
	if (PlayerCount < 0 ) PlayerCount = 0;
	UE_LOG(LogTemp, Log, TEXT("Joueur parti"));
	Super::Logout(Exiting);

}

void ALobbyGameMode::PreLogin(const FString& Options, const FString& Adress, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Adress, UniqueId, ErrorMessage);
	if (!CanHandleNewPlayer())
	{
		ErrorMessage = TEXT("Le lobby est plein ");
	}

}

bool ALobbyGameMode::CanHandleNewPlayer()
{
	if (PlayerCount >= MaxPlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("Refus de connexion : lobby plein"));
		return false; 
	}
	return true;
}
