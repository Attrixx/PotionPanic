#include "ItemSpawnLibrary.h"
#include "Engine/AssetManager.h"
#include "Engine/World.h"
#include "ItemActor.h"
#include "ItemAsset.h"

AItemActor* UItemSpawnLibrary::SpawnItemFromAsset(
	UObject* WorldContextObject,
	UItemAsset* ItemAsset,
	const FTransform& SpawnTransform,
	TSubclassOf<AItemActor> ItemClass)
{
	if (WorldContextObject == nullptr || ItemAsset == nullptr)
	{
		return nullptr;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (World == nullptr)
	{
		return nullptr;
	}

	UClass* ResolvedItemClass = ItemClass ? *ItemClass : AItemActor::StaticClass();
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AItemActor* SpawnedItem = World->SpawnActor<AItemActor>(ResolvedItemClass, SpawnTransform, SpawnParameters);
	if (SpawnedItem)
	{
		SpawnedItem->SetItemAsset(ItemAsset);
	}

	return SpawnedItem;
}

AItemActor* UItemSpawnLibrary::SpawnItemFromPrimaryAssetId(
	UObject* WorldContextObject,
	FPrimaryAssetId ItemAssetId,
	const FTransform& SpawnTransform,
	TSubclassOf<AItemActor> ItemClass)
{
	if (!ItemAssetId.IsValid())
	{
		return nullptr;
	}

	UItemAsset* ItemAsset = UAssetManager::Get().GetPrimaryAssetObject<UItemAsset>(ItemAssetId);
	if (ItemAsset == nullptr)
	{
		const FSoftObjectPath AssetPath = UAssetManager::Get().GetPrimaryAssetPath(ItemAssetId);
		ItemAsset = Cast<UItemAsset>(AssetPath.TryLoad());
	}

	return SpawnItemFromAsset(WorldContextObject, ItemAsset, SpawnTransform, ItemClass);
}

void UItemSpawnLibrary::DestroyItemActor(AItemActor* ItemActor, bool bPlayFeedback)
{
	if (ItemActor)
	{
		ItemActor->DestroyItem(bPlayFeedback);
	}
}
