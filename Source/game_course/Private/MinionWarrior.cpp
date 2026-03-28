// Fill out your copyright notice in the Description page of Project Settings.

#include "MinionWarrior.h"
#include "MinionWarriorAIController.h"

AMinionWarrior::AMinionWarrior()
{
	PrimaryActorTick.bCanEverTick = true;
	AIControllerClass = AMinionWarriorAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AMinionWarrior::BeginPlay()
{
	Super::BeginPlay();
}
