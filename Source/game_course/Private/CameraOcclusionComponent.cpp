// Fill out your copyright notice in the Description page of Project Settings.

#include "CameraOcclusionComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Engine/World.h"

UCameraOcclusionComponent::UCameraOcclusionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCameraOcclusionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!OcclusionMPC) return;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	// Cache player controller — only cast once
	if (!CachedPC)
	{
		if (APawn* OwnerPawn = Cast<APawn>(Owner))
		{
			CachedPC = Cast<APlayerController>(OwnerPawn->GetController());
		}
	}
	if (!CachedPC) return;

	const float Now = GetWorld()->GetTimeSeconds();
	const float Target = bWasOccluded ? OccludedOpacity : 1.f;
	const bool bOpacitySettled = FMath::IsNearlyEqual(CurrentOpacity, Target, 0.01f);

	// Skip trace when opacity has settled — only re-check at 10Hz
	if (bOpacitySettled && (Now - LastTraceTime) < 0.1f)
	{
		return;
	}
	LastTraceTime = Now;

	FVector CamLoc;
	FRotator CamRot;
	CachedPC->GetPlayerViewPoint(CamLoc, CamRot);

	TArray<FHitResult> Hits;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);
	GetWorld()->LineTraceMultiByChannel(Hits, CamLoc, Owner->GetActorLocation(),
		ECC_Camera, Params);

	const bool bOccluded = Hits.Num() > 0;
	const float NewTarget = bOccluded ? OccludedOpacity : 1.f;

	if (bOccluded != bWasOccluded || !FMath::IsNearlyEqual(CurrentOpacity, NewTarget, 0.001f))
	{
		CurrentOpacity = FMath::FInterpTo(CurrentOpacity, NewTarget, DeltaTime, FadeSpeed);
		UKismetMaterialLibrary::SetScalarParameterValue(
			GetWorld(), OcclusionMPC, OpacityParamName, CurrentOpacity);
	}

	bWasOccluded = bOccluded;
}
