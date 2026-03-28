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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mage|Animations")
	TArray<UAnimSequence*> HealCastAnimations;

	// Projectile Blueprint to spawn on attack
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mage|Combat")
	TSubclassOf<class AMinionMageProjectile> ProjectileClass;

	// Range at which the mage starts shooting
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mage|Combat")
	float AttackRange = 800.f;

	// Seconds between shots
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mage|Combat")
	float AttackCooldown = 2.0f;

	// Heal amount per cast
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mage|Healing")
	float HealAmount = 40.f;

	// Radius to find ally enemies to heal
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mage|Healing")
	float HealRange = 600.f;

	// Seconds between heals
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mage|Healing")
	float HealCooldown = 1.f;

	// Implement in Blueprint — spawn cast VFX on the mage and a heal VFX on the target
	UFUNCTION(BlueprintImplementableEvent, Category = "Mage|Healing")
	void OnHealCast(AActor* HealedTarget);

private:
	void UpdateAnimations();
	void SetAnimState(EMageAnimState NewState);
	void TryAttackPlayer();
	void TryHealAlly();

	EMageAnimState CurrentAnimState = EMageAnimState::Idle;
	bool bAttackOnCooldown = false;
	FTimerHandle AttackCooldownTimer;
	bool bHealOnCooldown = false;
	FTimerHandle HealCooldownTimer;
};
