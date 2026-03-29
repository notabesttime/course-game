// Fill out your copyright notice in the Description page of Project Settings.

#include "CameraOcclusionComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UCameraOcclusionComponent::UCameraOcclusionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCameraOcclusionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* Owner = GetOwner();
	if (!Owner) return;

	APlayerController* PC = Cast<APlayerController>(
		Cast<APawn>(Owner) ? Cast<APawn>(Owner)->GetController() : nullptr);
	if (!PC) return;

	// Camera location
	FVector CamLoc;
	FRotator CamRot;
	PC->GetPlayerViewPoint(CamLoc, CamRot);

	const FVector PlayerLoc = Owner->GetActorLocation();

	// Multi-trace so we catch everything between camera and player
	TArray<FHitResult> Hits;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);
	Params.bReturnPhysicalMaterial = false;

	GetWorld()->LineTraceMultiByChannel(Hits, CamLoc, PlayerLoc,
		ECC_Visibility, Params);

	// Build set of currently occluding meshes
	TSet<TWeakObjectPtr<UPrimitiveComponent>> CurrentOccluders;
	for (const FHitResult& Hit : Hits)
	{
		UPrimitiveComponent* Comp = Hit.GetComponent();
		if (!Comp) continue;
		// Only affect static meshes tagged for occlusion (have the param)
		CurrentOccluders.Add(Comp);
	}

	// Fade out new occluders, fade in meshes that are no longer occluding
	TArray<TWeakObjectPtr<UPrimitiveComponent>> ToRemove;

	for (auto& Pair : FadedMeshes)
	{
		UPrimitiveComponent* Comp = Pair.Key.Get();
		if (!Comp)
		{
			ToRemove.Add(Pair.Key);
			continue;
		}

		const bool bOccluding = CurrentOccluders.Contains(Pair.Key);
		const float Target = bOccluding ? OccludedOpacity : 1.f;
		Pair.Value = FMath::FInterpTo(Pair.Value, Target, DeltaTime, FadeSpeed);

		Comp->SetScalarParameterValueOnMaterials(OpacityParamName, Pair.Value);

		// Once fully restored, remove from the map
		if (!bOccluding && FMath::IsNearlyEqual(Pair.Value, 1.f, 0.01f))
		{
			Comp->SetScalarParameterValueOnMaterials(OpacityParamName, 1.f);
			ToRemove.Add(Pair.Key);
		}
	}

	for (auto& Key : ToRemove)
	{
		FadedMeshes.Remove(Key);
	}

	// Add newly occluding meshes that aren't tracked yet
	for (auto& WeakComp : CurrentOccluders)
	{
		if (!FadedMeshes.Contains(WeakComp))
		{
			FadedMeshes.Add(WeakComp, 1.f);
		}
	}
}
