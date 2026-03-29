// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnerManager.generated.h"

UCLASS()
class GAME_COURSE_API ASpawnerManager : public AActor
{
	GENERATED_BODY()

public:
	ASpawnerManager();

protected:
	virtual void BeginPlay() override;

	// Blueprint subclass of EnemySpawner to place in the world
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner Manager")
	TSubclassOf<class AEnemySpawner> SpawnerClass;

	// Seconds between each wave
	UPROPERTY(EditDefaultsOnly, Category = "Spawner Manager")
	float SpawnInterval = 15.f;

	// Radius around this actor to search for a valid nav point
	UPROPERTY(EditDefaultsOnly, Category = "Spawner Manager")
	float SearchRadius = 5000.f;

	// Minimum distance between any two spawners placed in the same wave
	UPROPERTY(EditDefaultsOnly, Category = "Spawner Manager")
	float MinSpawnerDistance = 1500.f;

private:
	void PlaceWave();
	void SpawnNextPending();

	FTimerHandle SpawnTimerHandle;
	FTimerHandle DripTimerHandle;
	TArray<FVector> PendingSpawnLocations;
	int32 NextWaveCount = 2;
	int32 WaveNumber = 1;
	int32 SpawnLimitBonus = 0;
};
