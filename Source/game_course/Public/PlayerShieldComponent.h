// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraComponent.h"
#include "PlayerShieldComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAME_COURSE_API UPlayerShieldComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerShieldComponent();

	// Call this when a mage is killed — activates shield for ShieldDuration seconds (or custom duration)
	void ActivateShield(float Duration = -1.f);

	// Returns true while shield is active — caller should skip damage application
	bool IsShieldActive() const { return bShieldActive; }
	float GetShieldTimeRemaining() const
	{
		if (!bShieldActive) return 0.f;
		return GetOwner()->GetWorldTimerManager().GetTimerRemaining(ShieldTimer);
	}

	// Niagara effect to attach to the player while shielded
	UPROPERTY(EditAnywhere, Category = "Shield")
	class UNiagaraSystem* ShieldEffect;

	// How long the shield lasts in seconds
	UPROPERTY(EditAnywhere, Category = "Shield")
	float ShieldDuration = 2.f;

private:
	void DeactivateShield();
	void ReplayEffect();

	bool bShieldActive = false;
	FTimerHandle ShieldTimer;
	FTimerHandle EffectReplayTimer;

	UPROPERTY()
	class UNiagaraComponent* ActiveEffectComp;
};
