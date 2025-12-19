// Fill out your copyright notice in the Description page of Project Settings.

#include "AlchemistBase.h"

DEFINE_LOG_CATEGORY_STATIC(PP_AlchemistBase, Log, All);

AAlchemistBase::AAlchemistBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AAlchemistBase::Server_SendIntent_Implementation(EIntentType Intent)
{
	switch (Intent)
	{
	case EIntentType::PickUpOrDrop: PickupOrDrop(); break;
	case EIntentType::Throw: Throw(); break;
	case EIntentType::Interact: Interact(); break;
	default:
	{
		UE_LOGFMT(PP_AlchemistBase, Warning, "Server received not implemented intent ({0}).", std::size_t(Intent));
	}
	break;
	}
}

void AAlchemistBase::PickupOrDrop()
{
	UE_LOGFMT(PP_AlchemistBase, Log, "PickupOrDrop");
}

void AAlchemistBase::Throw()
{
	UE_LOGFMT(PP_AlchemistBase, Log, "Throw");
}

void AAlchemistBase::Interact()
{
	UE_LOGFMT(PP_AlchemistBase, Log, "Interact");
}
