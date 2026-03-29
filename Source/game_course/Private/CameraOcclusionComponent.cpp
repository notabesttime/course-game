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

	APlayerController* PC = Cast<APlayerController>(
		Cast<APawn>(Owner) ? Cast<APawn>(Owner)->GetController() : nullptr);
	if (!PC) return;

	FVector CamLoc;
	FRotator CamRot;
	PC->GetPlayerViewPoint(CamLoc, CamRot);

	TArray<FHitResult> Hits;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);
	GetWorld()->LineTraceMultiByChannel(Hits, CamLoc, Owner->GetActorLocation(),
		ECC_Camera, Params);

	const bool bOccluded = Hits.Num() > 0;
	const float Target = bOccluded ? OccludedOpacity : 1.f;

	if (bOccluded != bWasOccluded || !FMath::IsNearlyEqual(CurrentOpacity, Target, 0.001f))
	{
		CurrentOpacity = FMath::FInterpTo(CurrentOpacity, Target, DeltaTime, FadeSpeed);
		UKismetMaterialLibrary::SetScalarParameterValue(
			GetWorld(), OcclusionMPC, OpacityParamName, CurrentOpacity);
	}

	bWasOccluded = bOccluded;
}
