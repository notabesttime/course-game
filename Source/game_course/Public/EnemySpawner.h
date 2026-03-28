// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseEnemy.h"
#include "EnemySpawner.generated.h"

UCLASS()
class GAME_COURSE_API AEnemySpawner : public ABaseEnemy
{
	GENERATED_BODY()

public:
	AEnemySpawner();

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

	// Maximum warriors allowed in the level at once
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Limits")
	int32 MaxWarriors = 15;

	// Maximum mages allowed in the level at once
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Limits")
	int32 MaxMages = 5;

	// Seconds between spawn attempts
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	float SpawnInterval = 3.0f;

private:
	void TrySpawn();

	int32 CountWarriors() const;
	int32 CountMages() const;

	FTimerHandle SpawnTimerHandle;
};
