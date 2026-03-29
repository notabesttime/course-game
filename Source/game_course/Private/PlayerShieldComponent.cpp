// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerShieldComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

UPlayerShieldComponent::UPlayerShieldComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPlayerShieldComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bShieldActive && ActiveEffectComp && !ActiveEffectComp->IsActive())
	{
		ActiveEffectComp->Activate(true);
	}
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

	// Spawn and attach effect if not already running
	if (ShieldEffect && !ActiveEffectComp)
	{
		ActiveEffectComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
			ShieldEffect,
			GetOwner()->GetRootComponent(),
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			false   // do NOT auto-destroy — we destroy it manually
		);
	}
}

void UPlayerShieldComponent::DeactivateShield()
{
	bShieldActive = false;

	if (ActiveEffectComp)
	{
		ActiveEffectComp->DestroyComponent();
		ActiveEffectComp = nullptr;
	}
}
