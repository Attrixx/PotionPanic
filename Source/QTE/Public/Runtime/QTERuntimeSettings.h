#pragma once

#include "CoreMinimal.h"
#include "Runtime/QTERuntime.h"
#include "UObject/Object.h"
#include "QTERuntimeSettings.generated.h"

class UQTEComponent;
class UQTEDefinitionDataAsset;

UCLASS(Abstract, EditInlineNew, BlueprintType)
class QTE_API UQTERuntimeSettings : public UObject
{
	GENERATED_BODY()

public:
	virtual TUniquePtr<FQTERuntime> CreateRuntime(UQTEComponent& OwnerComponent, UQTEDefinitionDataAsset& Definition) const PURE_VIRTUAL(UQTERuntimeSettings::CreateRuntime, return nullptr;);
	virtual EQTEType GetQTEType() const PURE_VIRTUAL(UQTERuntimeSettings::GetQTEType, return EQTEType::Press;);
};

UCLASS(EditInlineNew, BlueprintType)
class QTE_API UQTEPressRuntimeSettings final : public UQTERuntimeSettings
{
	GENERATED_BODY()

public:
	TUniquePtr<FQTERuntime> CreateRuntime(UQTEComponent& OwnerComponent, UQTEDefinitionDataAsset& Definition) const override;
	EQTEType GetQTEType() const override;
};

UCLASS(EditInlineNew, BlueprintType)
class QTE_API UQTEHoldRuntimeSettings final : public UQTERuntimeSettings
{
	GENERATED_BODY()

public:
	TUniquePtr<FQTERuntime> CreateRuntime(UQTEComponent& OwnerComponent, UQTEDefinitionDataAsset& Definition) const override;
	EQTEType GetQTEType() const override;
};

UCLASS(EditInlineNew, BlueprintType)
class QTE_API UQTEMashRuntimeSettings final : public UQTERuntimeSettings
{
	GENERATED_BODY()

public:
	TUniquePtr<FQTERuntime> CreateRuntime(UQTEComponent& OwnerComponent, UQTEDefinitionDataAsset& Definition) const override;
	EQTEType GetQTEType() const override;
};

UCLASS(EditInlineNew, BlueprintType)
class QTE_API UQTESequenceRuntimeSettings final : public UQTERuntimeSettings
{
	GENERATED_BODY()

public:
	TUniquePtr<FQTERuntime> CreateRuntime(UQTEComponent& OwnerComponent, UQTEDefinitionDataAsset& Definition) const override;
	EQTEType GetQTEType() const override;
};

UCLASS(EditInlineNew, BlueprintType)
class QTE_API UQTEChainRuntimeSettings final : public UQTERuntimeSettings
{
	GENERATED_BODY()

public:
	TUniquePtr<FQTERuntime> CreateRuntime(UQTEComponent& OwnerComponent, UQTEDefinitionDataAsset& Definition) const override;
	EQTEType GetQTEType() const override;
};
