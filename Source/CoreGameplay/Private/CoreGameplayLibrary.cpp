// Fill out your copyright notice in the Description page of Project Settings.

#include "CoreGameplayLibrary.h"

UActorComponent* UCoreGameplayLibrary::FindComponentInAttachChain(AActor* Actor, TSubclassOf<UActorComponent> ComponentClass)
{
	if (!ComponentClass)
	{
		return nullptr;
	}

	for (AActor* Current = Actor; Current; Current = Current->GetAttachParentActor())
	{
		if (UActorComponent* Component = Current->FindComponentByClass(ComponentClass))
		{
			return Component;
		}
	}

	return nullptr;
}

FLinearColor UCoreGameplayLibrary::MakeColorFromTag(FGameplayTag Tag)
{
	return MakeColorFromHash(HashTag(Tag));
}

FLinearColor UCoreGameplayLibrary::MakeColorFromTagContainer(const FGameplayTagContainer& Tags)
{
	// Combined with xor on purpose: a container is a set, so two containers holding the same tags
	// must come out the same colour whatever order they were built in. Duplicates would cancel
	// each other out, but a container never holds any.
	uint32 Hash = 0;
	for (const FGameplayTag& Tag : Tags)
	{
		Hash ^= HashTag(Tag);
	}

	return MakeColorFromHash(Hash);
}

uint32 UCoreGameplayLibrary::HashTag(const FGameplayTag& Tag)
{
	// Crc of the tag's own text: stable across runs, machines and builds, which is the whole point.
	return FCrc::StrCrc32(*Tag.GetTagName().ToString());
}

FLinearColor UCoreGameplayLibrary::MakeColorFromHash(uint32 Hash)
{
	// Full saturation and full value: the conversion below always leaves one of the three channels
	// sitting at Value, so exactly one of R, G, B comes out at 1 and the colour never looks washed
	// out or muddy. Only the hue carries the hash.
	const float Hue = static_cast<float>(Hash % 360u);

	return FLinearColor(Hue, 1.f, 1.f, 1.f).HSVToLinearRGB();
}
