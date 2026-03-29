// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "BaseCompanion.generated.h"

UENUM()
enum class ECompanionAnimState : uint8
{
	Idle,
	Walking,
	Running,
	Attacking
};

UCLASS(Abstract, Blueprintable)
class GAME_COURSE_API ABaseCompanion : public ABaseCharacter
{
	GENERATED_BODY()

public:
	ABaseCompanion();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// If player is farther than this the companion follows them, ignoring enemies
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Companion")
	float FollowDistance = 400.f;

	// Radius to scan for enemies
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Companion")
	float DetectionRadius = 600.f;

	// Distance from enemy at which the companion stops moving and attacks
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Companion")
	float AttackRange = 150.f;

	// Damage dealt per attack
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Companion")
	float AttackDamage = 15.f;

	// Radius around the companion to hit multiple enemies per attack
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Companion")
	float AttackRadius = 200.f;

	// Seconds between attacks
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Companion")
	float AttackCooldown = 1.0f;

	// Animations — set these in BP_ShinbiWolf Class Defaults
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Companion|Animations")
	UAnimSequence* IdleAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Companion|Animations")
	UAnimSequence* WalkAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Companion|Animations")
	UAnimSequence* RunAnimation;

	// Speed below which WalkAnimation plays instead of RunAnimation
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Companion|Animations")
	float WalkSpeedThreshold = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Companion|Animations")
	TArray<UAnimSequence*> AttackAnimations;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Companion|Sounds")
	USoundBase* AttackSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Companion|Sounds")
	USoundBase* DeathSound;

	// Called when the companion attacks — implement in Blueprint for VFX/animation
	UFUNCTION(BlueprintImplementableEvent, Category = "Companion")
	void OnCompanionAttack(AActor* Target);

	UFUNCTION()
	void OnHealthChanged(float NewValue, float OldValue, float MaxValue);

private:
	void UpdateBehavior();
	class ABaseEnemy* FindNearestEnemy() const;
	void PerformAttack(class ABaseEnemy* Target);

	UPROPERTY()
	class APlayerCharacter* PlayerOwner = nullptr;

	UPROPERTY()
	class ABaseEnemy* CurrentTarget = nullptr;

	void UpdateAnimations();
	void SetAnimState(ECompanionAnimState NewState);

	bool bAttackOnCooldown = false;
	FTimerHandle AttackCooldownTimer;

	ECompanionAnimState CurrentAnimState = ECompanionAnimState::Idle;

	float LastTargetSearchTime = -1.f;
};
