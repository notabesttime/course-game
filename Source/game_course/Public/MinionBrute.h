// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseEnemy.h"
#include "MinionBrute.generated.h"

UENUM()
enum class EBruteAnimState : uint8
{
	Idle,
	Running,
	Attacking
};

UCLASS()
class GAME_COURSE_API AMinionBrute : public ABaseEnemy
{
	GENERATED_BODY()

public:
	AMinionBrute();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Skin meshes — one is picked randomly on spawn
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brute|Appearance")
	TArray<USkeletalMesh*> SkinVariants;

	// Animations
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brute|Animations")
	UAnimSequence* IdleAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brute|Animations")
	UAnimSequence* RunAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brute|Animations")
	TArray<UAnimSequence*> AttackAnimations;

	// Sounds
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brute|Sounds")
	USoundBase* AttackSound;

	// Damage dealt to player per attack
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brute|Combat")
	float AttackDamage = 50.f;

	// Range at which the brute starts attacking
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brute|Combat")
	float AttackRange = 180.f;

	// Seconds between attacks
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brute|Combat")
	float AttackCooldown = 1.5f;

	virtual void OnDied() override;

private:
	void UpdateAnimations();
	void SetAnimState(EBruteAnimState NewState);
	void TryAttackPlayer();

	EBruteAnimState CurrentAnimState = EBruteAnimState::Idle;

	bool bAttackOnCooldown = false;
	FTimerHandle AttackCooldownTimer;
};
