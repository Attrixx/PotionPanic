#pragma once

#include "CoreMinimal.h"
#include "DistinguishComponent.h"
#include "ShaderDistinguishComponent.generated.h"

UCLASS()
class POTIONPANIC_API UShaderDistinguishComponent : public UDistinguishComponent
{
	GENERATED_BODY()
	
public:
	UShaderDistinguishComponent();

protected:
	void BeginPlay() override;

	void OnActivate() override;
	void OnDeactivate() override;

protected:
	UPROPERTY(EditAnywhere, Category = "Distinguish")
	UMaterialInterface* HighlightMaterial = nullptr;

private:
	UPROPERTY()
	class UStaticMeshComponent* CachedMesh = nullptr;

	UPROPERTY()
	TArray<UMaterialInterface*> OriginalMaterials;
};