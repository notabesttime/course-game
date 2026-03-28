// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseEnemy.h"
#include "MinionMage.generated.h"

UENUM()
enum class EMageAnimState : uint8
{
	Idle,
	Running,
	Casting
};

UCLASS()
class GAME_COURSE_API AMinionMage : public ABaseEnemy
{
	GENERATED_BODY()

public:
	AMinionMage();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Skin meshes — one is picked randomly on spawn
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mage|Appearance")
	TArray<USkeletalMesh*> SkinVariants;

	// Animations
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mage|Animations")
	UAnimSequence* IdleAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mage|Animations")
	UAnimSequence* RunAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mage|Animations")
	TArray<UAnimSequence*> CastAnimations;

	// Projectile Blueprint to spawn on attack
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mage|Combat")
	TSubclassOf<class AMinionMageProjectile> ProjectileClass;

	// Range at which the mage starts shooting
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mage|Combat")
	float AttackRange = 800.f;

	// Seconds between shots
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mage|Combat")
	float AttackCooldown = 2.0f;

private:
	void UpdateAnimations();
	void SetAnimState(EMageAnimState NewState);
	void TryAttackPlayer();

	EMageAnimState CurrentAnimState = EMageAnimState::Idle;
	bool bAttackOnCooldown = false;
	FTimerHandle AttackCooldownTimer;
};
