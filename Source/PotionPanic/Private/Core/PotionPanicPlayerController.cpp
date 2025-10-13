// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/PotionPanicPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "Core/PotionPanicCharacter.h"
#include "Kismet/KismetMathLibrary.h"

APotionPanicPlayerController::APotionPanicPlayerController()
{
}

void APotionPanicPlayerController::BeginPlay()
{
	Super::BeginPlay();

	PotionPanicCharacter = Cast<APotionPanicCharacter>(GetCharacter());

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(InputMappingContext, 0);

		if (UEnhancedInputUserSettings* UserSettings = Subsystem->GetUserSettings())
		{
			if (!UserSettings->IsMappingContextRegistered(InputMappingContext))
			{
				UserSettings->RegisterInputMappingContext(InputMappingContext);
			}
		}
	}
}

void APotionPanicPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction,		ETriggerEvent::Triggered, this, &ThisClass::Move);
		EnhancedInputComponent->BindAction(InteractAction,	ETriggerEvent::Triggered, this, &ThisClass::Interact);
		EnhancedInputComponent->BindAction(CarryAction,		ETriggerEvent::Triggered, this, &ThisClass::Carry);
	}
}

void APotionPanicPlayerController::Move(const FInputActionValue& Value)
{
	ACharacter* CurrentCharacter = GetCharacter();
	if (CurrentCharacter == nullptr) return;
	
	FVector2D MovementVector = Value.Get<FVector2D>();
	CurrentCharacter->AddMovementInput(FVector::ForwardVector, MovementVector.Y);
	CurrentCharacter->AddMovementInput(FVector::RightVector, MovementVector.X);
	
	const float RotationAngle = FMath::Atan2(-MovementVector.Y, MovementVector.X);
	const float TargetYaw = FMath::RadiansToDegrees(RotationAngle) + 90.f;
	const float CurrentYaw = CurrentCharacter->GetActorRotation().Yaw;
	const float DeltaYaw = UKismetMathLibrary::NormalizeAxis(TargetYaw - CurrentYaw);
	float NormalizedYawInput = FMath::Clamp(DeltaYaw, -1.f, 1.f);
	CurrentCharacter->AddControllerYawInput(NormalizedYawInput);
}

void APotionPanicPlayerController::Interact(const FInputActionValue& Value)
{
	if (PotionPanicCharacter == nullptr) return;

	PotionPanicCharacter->OnInteract();
}

void APotionPanicPlayerController::Carry(const FInputActionValue& Value)
{
	if (PotionPanicCharacter == nullptr) return;

	PotionPanicCharacter->OnCarry();
}
