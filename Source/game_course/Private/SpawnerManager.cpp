// Fill out your copyright notice in the Description page of Project Settings.

#include "SpawnerManager.h"
#include "EnemySpawner.h"
#include "GameHUD.h"
#include "GameTimerWidget.h"
#include "NavigationSystem.h"
#include "GameFramework/PlayerController.h"

ASpawnerManager::ASpawnerManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ASpawnerManager::BeginPlay()
{
	Super::BeginPlay();

	if (SpawnerClass)
	{
		GetWorldTimerManager().SetTimer(
			SpawnTimerHandle,
			this,
			&ASpawnerManager::PlaceWave,
			SpawnInterval,
			true
		);
	}
}

void ASpawnerManager::PlaceWave()
{
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys || !SpawnerClass)
	{
		return;
	}

	for (int32 i = 0; i < NextWaveCount; i++)
	{
		FNavLocation RandomLocation;
		if (NavSys->GetRandomReachablePointInRadius(GetActorLocation(), SearchRadius, RandomLocation))
		{
			GetWorld()->SpawnActor<AEnemySpawner>(SpawnerClass, RandomLocation.Location, FRotator::ZeroRotator);
		}
	}

	NextWaveCount *= 2;
	WaveNumber++;

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (AGameHUD* HUD = Cast<AGameHUD>(PC->GetHUD()))
		{
			if (UGameTimerWidget* TimerWidget = HUD->GetTimerWidget())
			{
				TimerWidget->StartFlash(WaveNumber);
			}
		}
	}
}
