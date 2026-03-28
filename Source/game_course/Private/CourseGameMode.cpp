// Fill out your copyright notice in the Description page of Project Settings.

#include "CourseGameMode.h"
#include "SpawnerManager.h"

void ACourseGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (SpawnerManagerClass)
	{
		GetWorld()->SpawnActor<ASpawnerManager>(SpawnerManagerClass, FVector::ZeroVector, FRotator::ZeroRotator);
	}
}
