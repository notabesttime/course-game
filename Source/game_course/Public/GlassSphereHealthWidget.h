// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GlassSphereHealthWidget.generated.h"

UCLASS()
class GAME_COURSE_API UGlassSphereHealthWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetHealthComponent(class UHealthComponent* InHealthComponent);

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
		int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FVector2D NativeGetDesiredSize() const override;

	// Radius of the sphere in pixels
	UPROPERTY(EditDefaultsOnly, Category = "Sphere")
	float SphereRadius = 50.f;

	// How strongly the liquid springs back to flat
	UPROPERTY(EditDefaultsOnly, Category = "Sphere|Wobble")
	float WobbleStiffness = 12.f;

	// Damping on the spring (higher = less oscillation)
	UPROPERTY(EditDefaultsOnly, Category = "Sphere|Wobble")
	float WobbleDamping = 4.5f;

	// How much player acceleration drives the wobble
	UPROPERTY(EditDefaultsOnly, Category = "Sphere|Wobble")
	float WobbleVelocityScale = 0.025f;

	// Maximum wobble displacement in pixels
	UPROPERTY(EditDefaultsOnly, Category = "Sphere|Wobble")
	float MaxWobble = 7.f;

private:
	UFUNCTION()
	void OnHealthChanged(float NewValue, float OldValue, float MaxValue);

	UPROPERTY()
	class UHealthComponent* HealthComponent;

	float HealthPercent = 1.f;

	// Tilt: horizontal slope of the liquid surface
	float WobbleTilt = 0.f;
	float WobbleTiltVel = 0.f;

	// Amplitude of the surface wave
	float WobbleAmp = 0.f;
	float WobbleAmpVel = 0.f;

	// Phase of the surface wave (advances over time)
	float WobblePhase = 0.f;

	FVector LastVelocity = FVector::ZeroVector;
};
