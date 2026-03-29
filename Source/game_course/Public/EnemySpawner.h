// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnemySpawner.generated.h"

class AMinionBrute;

UCLASS()
class GAME_COURSE_API AEnemySpawner : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AEnemySpawner();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// Maximum warriors allowed in the level at once
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Limits")
	int32 MaxWarriors = 20;

	// Maximum mages allowed in the level at once
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Limits")
	int32 MaxMages = 10;

	// Maximum brutes allowed in the level at once
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Limits")
	int32 MaxBrutes = 4;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Effect and sound played when this spawner first appears
	UPROPERTY(EditDefaultsOnly, Category = "Spawner|Appear")
	class UParticleSystem* AppearEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Spawner|Appear")
	USoundBase* AppearSound;

	UPROPERTY(EditDefaultsOnly, Category = "Spawner|Sounds")
	USoundBase* SpawnSound;

	UPROPERTY(EditDefaultsOnly, Category = "Spawner|Sounds")
	USoundBase* DeathSound;

	UPROPERTY(EditDefaultsOnly, Category = "Spawner|UI")
	TSubclassOf<class UHealthBarWidget> HealthBarWidgetClass;

	// Degrees per second to rotate around Z axis
	UPROPERTY(EditDefaultsOnly, Category = "Spawner")
	float SpinSpeed = 45.f;

	// Blueprint subclass of MinionWarrior to spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	TSubclassOf<class AMinionWarrior> WarriorClass;

	// Blueprint subclass of MinionMage to spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	TSubclassOf<class AMinionMage> MageClass;

	// Blueprint subclass of MinionBrute to spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	TSubclassOf<AMinionBrute> BruteClass;

	// Seconds between spawn attempts
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	float SpawnInterval = 3.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY()
	class UBaseAttributeSet* AttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	class UHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	UCapsuleComponent* CapsuleComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	class UWidgetComponent* HealthBarWidgetComponent;

public:
	// Live enemy counts — incremented on spawn, decremented in each minion's OnDied()
	static int32 LiveWarriorCount;
	static int32 LiveMageCount;
	static int32 LiveBruteCount;

	// Reset all static counters — must be called at session start (they survive PIE/OpenLevel)
	static void ResetCounters();

private:
	void TrySpawn();

	UFUNCTION()
	void OnHealthChanged(float NewValue, float OldValue, float MaxValue);

	FTimerHandle SpawnTimerHandle;
	static int32 TotalSpawnCount;
};
