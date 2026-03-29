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

	// Returns the currently hovered enemy or spawner (any damageable actor under the cursor)
	UFUNCTION(BlueprintPure, Category = "Combat")
	AActor* GetHoveredEnemy() const { return HoveredEnemy; }


protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// Input actions — create IA_Melee and IA_Ranged in editor, map to LMB/RMB
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraOcclusionComponent* CameraOcclusionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shield")
	class UPlayerShieldComponent* ShieldComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* MeleeInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* RangedInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* MoveInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* DebugDieInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* RestartInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* DebugShieldInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* TimeSlowInputAction;

	// Set these in BP_PlayerCharacter Class Defaults to the Blueprint ability subclasses
	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TSubclassOf<class UPlayerMeleeAbility> MeleeAbilityClass;

	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TSubclassOf<class UPlayerRangedAbility> RangedAbilityClass;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundBase* MeleeAttackSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundBase* RangedAttackSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundBase* DeathSound;

public:
	float GetLastAttackTime() const { return LastAttackTime; }
	UPlayerShieldComponent* GetShieldComponent() const { return ShieldComponent; }

private:
	void ActivateMelee();
	void ActivateRanged();
	void Move(const struct FInputActionValue& Value);
	void UpdateHoveredEnemy();
	void DebugDie();
	void QuickRestart();
	void DebugActivateShield();
	void ActivateTimeSlow();
	void DeactivateTimeSlow();

	UFUNCTION()
	void OnHealthChanged(float NewValue, float OldValue, float MaxValue);

	float LastAttackTime = -10.f;
	float LastHoverUpdateTime = -1.f;
	bool bTimeSlowActive = false;
	bool bForceDebugDeath = false;

	TArray<TEnumAsByte<EObjectTypeQuery>> HoverObjectTypes;

	FGameplayAbilitySpecHandle MeleeAbilityHandle;
	FGameplayAbilitySpecHandle RangedAbilityHandle;

	UPROPERTY()
	AActor* HoveredEnemy = nullptr;
};
