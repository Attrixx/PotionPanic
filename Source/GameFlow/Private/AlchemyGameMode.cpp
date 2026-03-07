// Fill out your copyright notice in the Description page of Project Settings.

#include "AlchemyGameMode.h"
#include <EngineUtils.h>

DEFINE_LOG_CATEGORY_STATIC(MS_AlchemyGameMode, Log, All);

void AAlchemyGameMode::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);

	if (auto* PC = Cast<APlayerController>(NewPlayer))
	{
		if (auto* TargetActor = GetViewTarget())
		{
			PC->ClientSetViewTarget(TargetActor);
		}
	}
}

AActor* AAlchemyGameMode::GetViewTarget()
{
	if (CachedViewTarget.IsValid())
		return CachedViewTarget.Get();

	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		if (It->ActorHasTag(ViewTargetTag))
		{
			CachedViewTarget = *It;
			return CachedViewTarget.Get();
		}
	}

	UE_LOGFMT(MS_AlchemyGameMode, Error, "Could not find Actor with tag '{0}'", ViewTargetTag);
	return nullptr;
}