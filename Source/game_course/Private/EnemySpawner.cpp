// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemySpawner.h"
#include "MinionWarrior.h"
#include "MinionMage.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"

AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessAI = EAutoPossessAI::Disabled;
}

void AEnemySpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	AddActorLocalRotation(FRotator(0.f, SpinSpeed * DeltaTime, 0.f));
}

void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	AddActorLocalRotation(FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f));

	if (AppearEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(AppearEffect, GetRootComponent());
	}

	if (AppearSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, AppearSound, GetActorLocation(), 0.5f);
	}

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

	// Spawn transform: same location as spawner, yaw only (no pitch/roll)
	FTransform SpawnTransform(FRotator(0.f, GetActorRotation().Yaw, 0.f), GetActorLocation());

	bool bSpawned = false;

	// Try to spawn a warrior if under the limit
	if (WarriorClass && Warriors < MaxWarriors)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		GetWorld()->SpawnActor<AMinionWarrior>(WarriorClass, SpawnTransform, Params);
		bSpawned = true;
	}

	// Try to spawn a mage if under the limit and ratio allows (1 mage per 2 warriors)
	if (MageClass && Mages < MaxMages && Mages * 2 < Warriors)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		GetWorld()->SpawnActor<AMinionMage>(MageClass, SpawnTransform, Params);
		bSpawned = true;
	}

	if (bSpawned && SpawnSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SpawnSound, GetActorLocation(), 0.5f);
	}
}
