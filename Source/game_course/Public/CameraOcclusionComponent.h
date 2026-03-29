// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CameraOcclusionComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAME_COURSE_API UCameraOcclusionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCameraOcclusionComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	// Opacity applied to occluding meshes (0 = invisible, 1 = fully opaque)
	UPROPERTY(EditAnywhere, Category = "Occlusion")
	float OccludedOpacity = 0.2f;

	// Speed at which opacity fades in/out
	UPROPERTY(EditAnywhere, Category = "Occlusion")
	float FadeSpeed = 8.f;

	// Name of the scalar material parameter to drive
	UPROPERTY(EditAnywhere, Category = "Occlusion")
	FName OpacityParamName = FName("Opacity");

private:
	// Meshes currently being faded, with their current opacity
	TMap<TWeakObjectPtr<UPrimitiveComponent>, float> FadedMeshes;
};
