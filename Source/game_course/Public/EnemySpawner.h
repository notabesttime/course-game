// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseEnemy.h"
#include "EnemySpawner.generated.h"

class AMinionBrute;

UCLASS()
class GAME_COURSE_API AEnemySpawner : public ABaseEnemy
{
	GENERATED_BODY()

public:
	AEnemySpawner();

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

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundBase* SpawnSound;

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

public:
	// Live enemy counts — incremented on spawn, decremented in each minion's OnDied()
	static int32 LiveWarriorCount;
	static int32 LiveMageCount;
	static int32 LiveBruteCount;

private:
	void TrySpawn();

	FTimerHandle SpawnTimerHandle;
	static int32 TotalSpawnCount;
};
