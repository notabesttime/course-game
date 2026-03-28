// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "PlayerMeleeAbility.generated.h"

UCLASS(Blueprintable)
class GAME_COURSE_API UPlayerMeleeAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UPlayerMeleeAbility();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	// Radius around the player to hit enemies
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	float MeleeRadius = 200.0f;

	// Damage dealt to each enemy in range
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	float DamageAmount = 25.0f;

	// Seconds before the ability can be used again
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	float CooldownDuration = 0.5f;

	// Montages to randomly pick from on each swing
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	TArray<UAnimMontage*> SwingMontages;

	// Called on activation — implement in Blueprint for visuals (VFX, sound, animation)
	UFUNCTION(BlueprintImplementableEvent, Category = "Ability")
	void OnMeleeActivated(FVector HitLocation, UAnimMontage* SelectedMontage);

	// Called for each enemy hit — implement in Blueprint to spawn hit effects
	UFUNCTION(BlueprintImplementableEvent, Category = "Ability")
	void OnMeleeHit(FVector EnemyLocation);

private:
	bool bOnCooldown = false;
	FTimerHandle CooldownTimer;

	TArray<FVector> ApplyDamageInRadius(AActor* AvatarActor) const;
};
