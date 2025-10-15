#include "DistinguishSystem/ShaderDistinguishComponent.h"

DEFINE_LOG_CATEGORY_STATIC(MS_ShaderDistinguishComponent, Log, All);

UShaderDistinguishComponent::UShaderDistinguishComponent()
{
}

void UShaderDistinguishComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// This component should always be provided with a material reference
	if (!HighlightMaterial)
	{
		UE_LOGFMT(MS_ShaderDistinguishComponent, Error, "ShaderDistinguishComponent on {0} has no DistinguishMaterialInstance set.", GetOwner()->GetName());
	}

	// This component is made to change the aspect of a mesh, so it should be attached to an actor with a mesh
	CachedMesh = GetOwner()->FindComponentByClass<UStaticMeshComponent>();
	if (!CachedMesh)
	{
		UE_LOGFMT(MS_ShaderDistinguishComponent, Error, "ShaderDistinguishComponent on {0} is not attached to an actor with a mesh.", GetOwner()->GetName());
	}
}

void UShaderDistinguishComponent::OnActivate()
{
	if (!CachedMesh || !HighlightMaterial)
		return;


	const int32 NumMats = CachedMesh->GetNumMaterials();
	OriginalMaterials.SetNum(NumMats);

	for (int32 i = 0; i < NumMats; ++i)
	{
		OriginalMaterials[i] = CachedMesh->GetMaterial(i);
		CachedMesh->SetMaterial(i, HighlightMaterial);
	}
}

void UShaderDistinguishComponent::OnDeactivate()
{
	if (!CachedMesh)
		return;

	const int32 NumMats = CachedMesh->GetNumMaterials();
	for (int32 i = 0; i < NumMats && i < OriginalMaterials.Num(); ++i)
	{
		if (OriginalMaterials[i])
			CachedMesh->SetMaterial(i, OriginalMaterials[i]);
	}
}
