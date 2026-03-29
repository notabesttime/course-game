// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerShieldComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

UPlayerShieldComponent::UPlayerShieldComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerShieldComponent::ReplayEffect()
{
	if (!bShieldActive || !ShieldEffect) return;

	if (ActiveEffectComp)
	{
		ActiveEffectComp->DestroyComponent();
		ActiveEffectComp = nullptr;
	}

	ActiveEffectComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
		ShieldEffect,
		GetOwner()->GetRootComponent(),
		NAME_None,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		EAttachLocation::SnapToTarget,
		false
	);
}

void UPlayerShieldComponent::ActivateShield(float Duration /*= -1.f*/)
{
	// Use custom duration if provided, otherwise use ShieldDuration
	float EffectiveDuration = (Duration >= 0.f) ? Duration : ShieldDuration;

	// If already active, extend by remaining time rather than resetting
	if (bShieldActive)
	{
		float Remaining = GetOwner()->GetWorldTimerManager().GetTimerRemaining(ShieldTimer);
		EffectiveDuration += FMath::Max(Remaining, 0.f);
	}

	bShieldActive = true;

	GetOwner()->GetWorldTimerManager().SetTimer(
		ShieldTimer, this, &UPlayerShieldComponent::DeactivateShield, EffectiveDuration, false);

	// Spawn effect and start replay timer (replays every 2 seconds while shield is active)
	if (ShieldEffect)
	{
		if (!ActiveEffectComp)
		{
			ActiveEffectComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
				ShieldEffect,
				GetOwner()->GetRootComponent(),
				NAME_None,
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				EAttachLocation::SnapToTarget,
				false
			);
		}

		GetOwner()->GetWorldTimerManager().SetTimer(
			EffectReplayTimer, this, &UPlayerShieldComponent::ReplayEffect, 2.f, true);
	}
}

void UPlayerShieldComponent::DeactivateShield()
{
	bShieldActive = false;

	GetOwner()->GetWorldTimerManager().ClearTimer(EffectReplayTimer);

	if (ActiveEffectComp)
	{
		ActiveEffectComp->DestroyComponent();
		ActiveEffectComp = nullptr;
	}
}
