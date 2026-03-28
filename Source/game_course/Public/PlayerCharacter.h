// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "GameplayAbilitySpecHandle.h"
#include "PlayerCharacter.generated.h"

class ABaseEnemy;

UCLASS()
class GAME_COURSE_API APlayerCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();

	UFUNCTION(BlueprintPure, Category = "Combat")
	ABaseEnemy* GetHoveredEnemy() const { return HoveredEnemy; }

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// Input actions — create IA_Melee and IA_Ranged in editor, map to LMB/RMB
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* MeleeInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* RangedInputAction;

	// Set these in BP_PlayerCharacter Class Defaults to the Blueprint ability subclasses
	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TSubclassOf<class UPlayerMeleeAbility> MeleeAbilityClass;

	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TSubclassOf<class UPlayerRangedAbility> RangedAbilityClass;

private:
	void ActivateMelee();
	void ActivateRanged();
	void UpdateHoveredEnemy();

	FGameplayAbilitySpecHandle MeleeAbilityHandle;
	FGameplayAbilitySpecHandle RangedAbilityHandle;

	UPROPERTY()
	ABaseEnemy* HoveredEnemy = nullptr;
};
