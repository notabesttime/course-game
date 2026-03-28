// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CourseGameMode.generated.h"

UCLASS()
class GAME_COURSE_API ACourseGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	// Set this to BP_SpawnerManager (or a subclass) in the GameMode's Class Defaults
	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	TSubclassOf<class ASpawnerManager> SpawnerManagerClass;
};
