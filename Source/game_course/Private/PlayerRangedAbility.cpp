// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerRangedAbility.h"
#include "PlayerAbilityProjectile.h"
#include "PlayerCharacter.h"
#include "GameFramework/PlayerController.h"

UPlayerRangedAbility::UPlayerRangedAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UPlayerRangedAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	return !bOnCooldown && Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UPlayerRangedAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AActor* AvatarActor = ActorInfo->AvatarActor.Get();
	if (!AvatarActor || !ProjectileClass)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	// Fire toward hovered enemy if available, otherwise toward mouse cursor
	FVector FireDirection = AvatarActor->GetActorForwardVector();
	if (APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(AvatarActor))
	{
		if (AActor* Target = PlayerChar->GetHoveredEnemy())
		{
			FVector ToTarget = Target->GetActorLocation() - AvatarActor->GetActorLocation();
			FireDirection = ToTarget.GetSafeNormal();
		}
		else if (APlayerController* PC = Cast<APlayerController>(PlayerChar->GetController()))
		{
			FHitResult HitResult;
			if (PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
			{
				FVector ToMouse = HitResult.Location - AvatarActor->GetActorLocation();
				ToMouse.Z = 0.f;
				FireDirection = ToMouse.GetSafeNormal();
			}
		}
	}

	AvatarActor->SetActorRotation(FireDirection.Rotation());

	FVector SpawnLocation = AvatarActor->GetActorLocation() + FireDirection * SpawnOffset;
	FRotator SpawnRotation = FireDirection.Rotation();

	APlayerAbilityProjectile::SpawnOrReuse(
		GetWorld(),
		ProjectileClass,
		SpawnLocation,
		SpawnRotation,
		AvatarActor,
		Cast<APawn>(AvatarActor)
	);

	OnRangedFired(SpawnLocation, FireDirection);

	bOnCooldown = true;
	AvatarActor->GetWorldTimerManager().SetTimer(
		CooldownTimer,
		[this]() { bOnCooldown = false; },
		CooldownDuration,
		false
	);

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}
