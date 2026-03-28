// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemySpawner.h"
#include "MinionWarrior.h"
#include "MinionMage.h"
#include "Kismet/GameplayStatics.h"

AEnemySpawner::AEnemySpawner()
{
	// Disable AI possession — spawner does not move
	AutoPossessAI = EAutoPossessAI::Disabled;
}

void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(
		SpawnTimerHandle,
		this,
		&AEnemySpawner::TrySpawn,
		SpawnInterval,
		true
	);
}

int32 AEnemySpawner::CountWarriors() const
{
	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMinionWarrior::StaticClass(), Found);
	return Found.Num();
}

int32 AEnemySpawner::CountMages() const
{
	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMinionMage::StaticClass(), Found);
	return Found.Num();
}

void AEnemySpawner::TrySpawn()
{
	const int32 Warriors = CountWarriors();
	const int32 Mages = CountMages();

	// Try to spawn a warrior if under the limit
	if (WarriorClass && Warriors < MaxWarriors)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		GetWorld()->SpawnActor<AMinionWarrior>(WarriorClass, GetActorTransform(), Params);
	}

	// Try to spawn a mage if under the limit and ratio allows (1 mage per 2 warriors)
	if (MageClass && Mages < MaxMages && Mages * 2 < Warriors)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		GetWorld()->SpawnActor<AMinionMage>(MageClass, GetActorTransform(), Params);
	}
}
