#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/InteractionInterface.h"
#include "StationComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnBeginProcessDelegate, APawn*, TObjectPtr<AActor>);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnEndProcessDelegate, APawn*, TSubclassOf<AActor>);

class UStrategyInterface;

USTRUCT(BlueprintType)
struct FInputItemGroup
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<TSubclassOf<AActor>, int32> Counts;

	void Clear()
	{
		Counts.Empty();
	}

	void Add(TSubclassOf<AActor> Item)
	{
		int32& Value = Counts.FindOrAdd(Item);
		++Value;
	}

	bool operator==(const FInputItemGroup& Other) const
	{
		if (Counts.Num() != Other.Counts.Num())
			return false;

		for (const auto& KVP : Counts)
		{
			const int32* OtherVal = Other.Counts.Find(KVP.Key);
			if (!OtherVal || *OtherVal != KVP.Value)
				return false;
		}
		return true;
	}

	// DEBUG
	FString Print()
	{
		FString S;
		for (const auto& P : Counts)
		{
			if (!S.IsEmpty())
				S += TEXT(", ");

			S += FString::Printf(
				TEXT("(%s : %d)"),
				P.Key ? *P.Key->GetName() : TEXT("None"),
				P.Value
			);
		}
		return S;
	}
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UStationComponent : public UActorComponent, public IInteractionInterface
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere)
	FText Name;

	FOnBeginProcessDelegate OnBeginProcess;
	FOnEndProcessDelegate OnEndProcess;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FInputItemGroup> InputItems;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<UObject>> Strategies;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TSubclassOf<AActor>> OutputItems;

protected:

	TObjectPtr<AActor> GetItemOnSocket();

public:

	void Interact(APawn* Instigator) override;

	UFUNCTION(BlueprintCallable)
	void StartProcessItem(APawn* Instigator);

	void Store(TSubclassOf<AActor> Item);

private:

	FInputItemGroup InternalStorage;
};
