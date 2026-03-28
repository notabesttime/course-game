// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "PlayerRangedAbility.generated.h"

UCLASS(Blueprintable)
class GAME_COURSE_API UPlayerRangedAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UPlayerRangedAbility();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	// Blueprint subclass of AbilityProjectile to spawn
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	TSubclassOf<class APlayerAbilityProjectile> ProjectileClass;

	// Seconds before the ability can be used again
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	float CooldownDuration = 1.0f;

	// Spawn offset forward from the player so the projectile doesn't hit them
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	float SpawnOffset = 80.0f;

	// Called on activation — implement in Blueprint for visuals (cast animation, muzzle VFX)
	UFUNCTION(BlueprintImplementableEvent, Category = "Ability")
	void OnRangedFired(FVector SpawnLocation, FVector Direction);

private:
	bool bOnCooldown = false;
	FTimerHandle CooldownTimer;
};
