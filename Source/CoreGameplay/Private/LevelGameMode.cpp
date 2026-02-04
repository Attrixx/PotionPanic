// Fill out your copyright notice in the Description page of Project Settings.

#include "LevelGameMode.h"
#include <EngineUtils.h>

DEFINE_LOG_CATEGORY_STATIC(PP_LevelGameMode, Log, All);

void ALevelGameMode::OnPostLogin(AController* NewPlayer)
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

AActor* ALevelGameMode::GetViewTarget()
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
	
	UE_LOGFMT(PP_LevelGameMode, Error, "Could not find Actor with tag '{0}'", ViewTargetTag);
	return nullptr;
}
