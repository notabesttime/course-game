// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseEnemy.h"
#include "MinionWarrior.generated.h"

UENUM()
enum class EWarriorAnimState : uint8
{
	Idle,
	Running,
	Attacking
};

UCLASS()
class GAME_COURSE_API AMinionWarrior : public ABaseEnemy
{
	GENERATED_BODY()

public:
	AMinionWarrior();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Skin meshes — one is picked randomly on spawn
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Warrior|Appearance")
	TArray<USkeletalMesh*> SkinVariants;

	// Animations
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Warrior|Animations")
	UAnimSequence* IdleAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Warrior|Animations")
	UAnimSequence* RunAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Warrior|Animations")
	TArray<UAnimSequence*> AttackAnimations;

	// Sounds
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Warrior|Sounds")
	USoundBase* AttackSound;

	virtual void OnDied() override;

	// Damage dealt to player per attack
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Warrior|Combat")
	float AttackDamage = 5.f;

	// Range at which the warrior starts attacking
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Warrior|Combat")
	float AttackRange = 150.f;

	// Seconds between attacks
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Warrior|Combat")
	float AttackCooldown = 0.5f;

private:
	void UpdateAnimations();
	void SetAnimState(EWarriorAnimState NewState);
	void TryAttackPlayer();

	EWarriorAnimState CurrentAnimState = EWarriorAnimState::Idle;

	bool bAttackOnCooldown = false;
	FTimerHandle AttackCooldownTimer;

	UPROPERTY()
	class APlayerCharacter* CachedPlayer = nullptr;
};
