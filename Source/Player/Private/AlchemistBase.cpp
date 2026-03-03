// Fill out your copyright notice in the Description page of Project Settings.

#include "AlchemistBase.h"
#include "AlchemistMovementComponent.h"
#include "HolderComponent.h"
#include "CarriableComponent.h"
#include "Interactable.h"
#include <Components/CapsuleComponent.h>
#include <EnhancedInputComponent.h>
#include <EnhancedInputSubsystems.h>

DEFINE_LOG_CATEGORY_STATIC(MS_AlchemistBase, Log, All);

AAlchemistBase::AAlchemistBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UAlchemistMovementComponent>(CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	HolderComponent = CreateDefaultSubobject<UHolderComponent>(TEXT("Holder Component"));
	HolderComponent->SetupAttachment(GetMesh());

	CapsuleOverlapComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule Component"));
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
	SetActorTickEnabled(false);

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		auto* LocalPlayer = PlayerController->GetLocalPlayer();
		if (auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
		{
			if (MappingContext)
			{
				Subsystem->AddMappingContext(MappingContext, 0);
			}
		}
	}

	if (CapsuleOverlapComponent)
	{
		CapsuleOverlapComponent->OnComponentBeginOverlap.AddDynamic(this, &AAlchemistBase::Capsule_OnBeginOverlap);
		CapsuleOverlapComponent->OnComponentEndOverlap.AddDynamic(this, &AAlchemistBase::Capsule_OnEndOverlap);
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

void AAlchemistBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateBestComponents();
}

void AAlchemistBase::UpdateBestComponents()
{
	float BestInteractableScore = std::numeric_limits<float>::lowest();
	float BestCarriableScore = std::numeric_limits<float>::lowest();

	TScriptInterface<IInteractable> LocalBestInteractable = nullptr;
	UCarriableComponent* LocalBestCarriable = nullptr;

	for (auto [Actor, _] : OverlappedActors)
	{
		FVector ToActor = Actor->GetActorLocation() - GetActorLocation();
		float Dot = FVector::DotProduct(GetActorForwardVector(), ToActor.GetSafeNormal());
		float DistToActor = ToActor.Length();
		float Score = Dot - DistToActor / CapsuleOverlapComponent->GetScaledCapsuleRadius();

		if (auto* Carriable = Actor->GetComponentByClass<UCarriableComponent>())
		{
			if (Score > BestCarriableScore)
			{
				BestCarriableScore = Score;
				LocalBestCarriable = Carriable;
			}
		}

		if (Actor->Implements<UInteractable>())
		{
			if (Score > BestInteractableScore)
			{
				BestInteractableScore = Score;
				LocalBestInteractable = Actor;
			}
		}
	}

	if (BestInteractable != LocalBestInteractable)
	{
		LocalBestInteractable = std::exchange(BestInteractable, LocalBestInteractable);
		if (BestInteractable)
		{
			// TODO: Enable effects on BestInteractable
		}
		if (LocalBestInteractable)
		{
			// TODO: Disable effects on LocalBestInteractable
		}
	}

	if (BestCarriable != LocalBestCarriable)
	{
		{
			auto Temp = BestCarriable.Get();
			BestCarriable = LocalBestCarriable;
			LocalBestInteractable = Temp;
		}

		if (BestCarriable.IsValid())
		{
			// TODO: Enable effects on BestCarriable
		}
		if (LocalBestCarriable)
		{
			// TODO: Disable effects on LocalBestCarriable
		}
	}
}

void AAlchemistBase::Capsule_OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                            bool bFromSweep, const FHitResult& SweepResult)
{
	FOverlappedActor* OverlappedActor = OverlappedActors.FindByPredicate([OtherActor](const FOverlappedActor& OverlappedActor)
	{
		return OverlappedActor.Actor == OtherActor;
	});

	if (!OverlappedActor)
		OverlappedActor = &OverlappedActors.Emplace_GetRef(OtherActor, 0);

	++OverlappedActor->NbOccurrences;

	if (OverlappedActors.Num() > 1)
		SetActorTickEnabled(true);
	else
		UpdateBestComponents();
}

void AAlchemistBase::Capsule_OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	int32 OverlappedActorIndex = OverlappedActors.IndexOfByPredicate([OtherActor](const FOverlappedActor& OverlappedActor)
	{
		return OverlappedActor.Actor == OtherActor;
	});

	if (OverlappedActorIndex == INDEX_NONE)
		return;

	auto& OverlappedActor = OverlappedActors[OverlappedActorIndex];
	if (--OverlappedActor.NbOccurrences == 0)
	{
		OverlappedActors.RemoveAtSwap(OverlappedActorIndex);

		if (OverlappedActors.Num() <= 1)
		{
			SetActorTickEnabled(false);
			UpdateBestComponents();
		}
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
	if (BestInteractable)
		Server_Interact();
}

void AAlchemistBase::Input_PickupOrDrop()
{
	if (!HolderComponent)
		return;

	if (HolderComponent->GetCarriable())
		Server_Drop();
	else if (BestCarriable.IsValid())
		Server_Pickup();
}

void AAlchemistBase::Input_Throw()
{
	if (HolderComponent && HolderComponent->GetCarriable())
		Server_Throw();
}

void AAlchemistBase::Server_Interact_Implementation()
{
	if (BestInteractable)
	{
		// TODO: Check if this works (not using IInteractable::Execute_Interact??)
		BestInteractable->Interact(Cast<APlayerController>(Controller));
	}
}

void AAlchemistBase::Server_Pickup_Implementation()
{
	if (HolderComponent && !HolderComponent->GetCarriable())
	{
		HolderComponent->Replace(BestCarriable.Get());
	}
}

void AAlchemistBase::Server_Drop_Implementation()
{
	if (HolderComponent && HolderComponent->GetCarriable())
	{
		auto* Carriable = HolderComponent->Replace(nullptr);
		if (Carriable) Carriable->SnapToGround();
	}
}

void AAlchemistBase::Server_Throw_Implementation()
{
	auto* Carriable = HolderComponent->Replace(nullptr);
	if (Carriable)
	{
		if (Carriable->CanBeThrown())
		{
			Carriable->Throw(GetActorForwardVector() * ThrowForce);
		}
		else // Drop if can't throw
		{
			Carriable->SnapToGround();
		}
	}
}
