// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerMeleeAbility.h"
#include "BaseEnemy.h"
#include "EnemySpawner.h"
#include "CombatAttributeUtils.h"
#include "Engine/OverlapResult.h"
#include "Animation/AnimMontage.h"

UPlayerMeleeAbility::UPlayerMeleeAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UPlayerMeleeAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	return !bOnCooldown && Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UPlayerMeleeAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AActor* AvatarActor = ActorInfo->AvatarActor.Get();
	if (!AvatarActor)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	TArray<FVector> HitLocations = ApplyDamageInRadius(AvatarActor);

	for (const FVector& Loc : HitLocations)
	{
		OnMeleeHit(Loc);
	}

	FVector HitLocation = HitLocations.Num() > 0 ? HitLocations[0] : AvatarActor->GetActorLocation();

	UAnimMontage* SelectedMontage = nullptr;
	if (SwingMontages.Num() > 0)
	{
		SelectedMontage = SwingMontages[FMath::RandRange(0, SwingMontages.Num() - 1)];
	}

	OnMeleeActivated(HitLocation, SelectedMontage);

	bOnCooldown = true;
	AvatarActor->GetWorldTimerManager().SetTimer(
		CooldownTimer,
		[this]() { bOnCooldown = false; },
		CooldownDuration,
		false
	);

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

TArray<FVector> UPlayerMeleeAbility::ApplyDamageInRadius(AActor* AvatarActor) const
{
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(AvatarActor);

	AvatarActor->GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		AvatarActor->GetActorLocation(),
		FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn),
		FCollisionShape::MakeSphere(MeleeRadius),
		Params
	);

	TArray<FVector> HitLocations;
	TSet<AActor*> DamagedActors;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* HitActor = Overlap.GetActor();
		if (!HitActor || (!HitActor->IsA<ABaseEnemy>() && !HitActor->IsA<AEnemySpawner>()) || DamagedActors.Contains(HitActor))
		{
			continue;
		}

		if (CombatAttributeUtils::ApplyHealthDelta(HitActor, -DamageAmount))
		{
			DamagedActors.Add(HitActor);
			HitLocations.Add(HitActor->GetActorLocation());
		}
	}

	return HitLocations;
}
