// Fill out your copyright notice in the Description page of Project Settings.

#include "AlchemistBase.h"
#include "AlchemistMovementComponent.h"
#include "HolderComponent.h"
#include "Interactable.h"
#include "Carriable.h"
#include <Components/CapsuleComponent.h>
#include <EnhancedInputComponent.h>
#include <EnhancedInputSubsystems.h>

DEFINE_LOG_CATEGORY_STATIC(MS_AlchemistBase, Log, All);

AAlchemistBase::AAlchemistBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UAlchemistMovementComponent>(CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	HolderComponent = CreateDefaultSubobject<UHolderComponent>(TEXT("Holder Component"));
	HolderComponent->SetupAttachment(GetMesh());

	CapsuleOverlapComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule Overlap Component"));
	CapsuleOverlapComponent->SetupAttachment(RootComponent);
	CapsuleOverlapComponent->SetGenerateOverlapEvents(true);
}

void AAlchemistBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (HolderComponent && GetMesh() && GetMesh()->DoesSocketExist(HolderParentSocket))
	{
		HolderComponent->AttachToComponent(GetMesh(),
			FAttachmentTransformRules::KeepRelativeTransform,
			HolderParentSocket);
	}
}

void AAlchemistBase::BeginPlay()
{
	Super::BeginPlay();
	
	CapsuleOverlapComponent->OnComponentBeginOverlap.AddDynamic(this, &AAlchemistBase::Capsule_OnBeginOverlap);
	CapsuleOverlapComponent->OnComponentEndOverlap.AddDynamic(this, &AAlchemistBase::Capsule_OnEndOverlap);

	GetWorldTimerManager().SetTimer(InRangeSortTimerHandle,
		[this]
		{
			SortInRangeInfos();

			if (!IsLocallyControlled())
				return;

			// TODO: Replace this by actual effects

#if WITH_EDITOR // GetActorGuid is WITH_EDITOR only

			AActor* BestInteractable = GetBestInteractable();
			GEngine->AddOnScreenDebugMessage(
				int32(GetActorGuid().A),
				InRangeInfosSortInterval * 2,
				FColor::Cyan,
				FString::Format(
					TEXT("{0} Best Interactable: {1}"),
					FStringFormatOrderedArguments{
						GetName(),
						BestInteractable ? BestInteractable->GetName() : "None"
					}));

			AActor* BestCarriable = GetBestCarriable();
			GEngine->AddOnScreenDebugMessage(
				int32(GetActorGuid().B),
				InRangeInfosSortInterval * 2,
				FColor::Cyan,
				FString::Format(
					TEXT("{0} Best Carriable: {1}"),
					FStringFormatOrderedArguments{
						GetName(),
						BestCarriable ? BestCarriable->GetName() : "None"
					}));
#endif
		},
		InRangeInfosSortInterval,
		true);
}

void AAlchemistBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	CapsuleOverlapComponent->OnComponentBeginOverlap.RemoveAll(this);
	CapsuleOverlapComponent->OnComponentEndOverlap.RemoveAll(this);

	GetWorldTimerManager().ClearTimer(InRangeSortTimerHandle);
}

void AAlchemistBase::PawnClientRestart()
{
	Super::PawnClientRestart();

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (MappingContext)
			{
				Subsystem->AddMappingContext(MappingContext, 0);
			}
		}
	}
}

void AAlchemistBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	auto* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EIC)
	{
		UE_LOGFMT(MS_AlchemistBase, Error, "Cannot bind input on null Enhanced Input Component");
		return;
	}

	EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAlchemistBase::Input_Move);
	EIC->BindAction(DashAction, ETriggerEvent::Started, this, &AAlchemistBase::Input_Dash);
	EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &AAlchemistBase::Input_Interact);
	EIC->BindAction(PickupOrDropAction, ETriggerEvent::Started, this, &AAlchemistBase::Input_PickupOrDrop);
	EIC->BindAction(ThrowAction, ETriggerEvent::Started, this, &AAlchemistBase::Input_Throw);
}

void AAlchemistBase::SortInRangeInfos()
{
	InRangeInfos.RemoveAll([this](FInRangeInfo& InRangeInfo)
	{
		if (!InRangeInfo.Actor.IsValid())
			return true;

		FVector ToActor = InRangeInfo.Actor->GetActorLocation() - GetActorLocation();
		float Dot = FVector::DotProduct(GetActorForwardVector(), ToActor.GetSafeNormal());
		float DistToActor = ToActor.Length();
		InRangeInfo.Score = Dot - DistToActor / CapsuleOverlapComponent->GetScaledCapsuleRadius();
		return false;
	});

	InRangeInfos.Sort([](const FInRangeInfo& Left, const FInRangeInfo& Right)
	{
		// The best (higher) score should be first in the array
		return Left.Score > Right.Score;
	});
}

AActor* AAlchemistBase::GetBestInteractable() const
{
	const FInRangeInfo* InfoPtr = InRangeInfos.FindByPredicate([this](const FInRangeInfo& Info)
	{
		return Info.Actor->Implements<UInteractable>();
	});

	return InfoPtr ? InfoPtr->Actor.Get() : nullptr;
}

AActor* AAlchemistBase::GetBestCarriable() const
{
	const FInRangeInfo* InfoPtr = InRangeInfos.FindByPredicate([this](const FInRangeInfo& Info)
	{
		return Info.Actor->Implements<UCarriable>();
	});

	return InfoPtr ? InfoPtr->Actor.Get() : nullptr;
}

void AAlchemistBase::Capsule_OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                            bool bFromSweep, const FHitResult& SweepResult)
{
	FInRangeInfo* InRangeInfo = InRangeInfos.FindByPredicate([OtherActor](const FInRangeInfo& Info)
	{
		return Info.Actor == OtherActor;
	});

	if (!InRangeInfo)
		InRangeInfo = &InRangeInfos.Emplace_GetRef(OtherActor);

	++InRangeInfo->NbOccurrences;
}

void AAlchemistBase::Capsule_OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	int32 InfoIndex = InRangeInfos.IndexOfByPredicate([OtherActor](const FInRangeInfo& Info)
	{
		return Info.Actor == OtherActor;
	});

	if (InfoIndex == INDEX_NONE)
		return;

	auto& InRangeInfo = InRangeInfos[InfoIndex];
	if (--InRangeInfo.NbOccurrences == 0)
	{
		InRangeInfos.RemoveAt(InfoIndex);
	}
}

void AAlchemistBase::Input_Move(const FInputActionValue& Value)
{
	auto Axis2D = Value.Get<FInputActionValue::Axis2D>();

	FRotator CamRotation;
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (PlayerController->PlayerCameraManager)
		{
			CamRotation = PlayerController->PlayerCameraManager->GetCameraRotation();
		}
	}

	// Movement is based on camera forward and right vectors
	FVector RotatedInput = CamRotation.RotateVector({Axis2D.Y, Axis2D.X, 0});
	FVector MovementInput = FVector::VectorPlaneProject(RotatedInput, GetActorUpVector()).GetSafeNormal();
	AddMovementInput(MovementInput);
}

void AAlchemistBase::Input_Dash()
{
	if (auto* AMC = Cast<UAlchemistMovementComponent>(GetCharacterMovement()))
	{
		AMC->Dash();
	}
}

void AAlchemistBase::Input_Interact()
{
	if (GetBestInteractable())
		Server_Interact();
}

void AAlchemistBase::Input_PickupOrDrop()
{
	if (HolderComponent->GetCarriable())
		Server_Drop();
	else if (GetBestCarriable())
		Server_Pickup();
}

void AAlchemistBase::Input_Throw()
{
	if (HolderComponent->GetCarriable())
		Server_Throw();
}

void AAlchemistBase::Server_Interact_Implementation()
{
	if (AActor* Interactable = GetBestInteractable())
	{
		IInteractable::Execute_Interact(Interactable, this);
	}
}

void AAlchemistBase::Server_Pickup_Implementation()
{
	HolderComponent->TryPickup(GetBestCarriable());
}

void AAlchemistBase::Server_Drop_Implementation()
{
	HolderComponent->Release();
}

void AAlchemistBase::Server_Throw_Implementation()
{
	HolderComponent->Release(GetActorForwardVector() * ThrowForce);
}

AAlchemistBase::FInRangeInfo::FInRangeInfo(AActor* Actor)
	: Actor(Actor)
	, NbOccurrences(0)
	, Score(std::numeric_limits<float>::min())
{
}
