// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "CameraOcclusionComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAME_COURSE_API UCameraOcclusionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCameraOcclusionComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	// Assign MPC_Occlusion here in the Blueprint defaults
	UPROPERTY(EditAnywhere, Category = "Occlusion")
	UMaterialParameterCollection* OcclusionMPC;

	// Name of the scalar parameter in the MPC
	UPROPERTY(EditAnywhere, Category = "Occlusion")
	FName OpacityParamName = FName("FadeOpacity");

	// Opacity applied while occluding (0=invisible, 1=opaque)
	UPROPERTY(EditAnywhere, Category = "Occlusion")
	float OccludedOpacity = 0.2f;

	// Fade speed
	UPROPERTY(EditAnywhere, Category = "Occlusion")
	float FadeSpeed = 8.f;

private:
	float CurrentOpacity = 1.f;
	bool bWasOccluded = false;
};
