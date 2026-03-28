// Fill out your copyright notice in the Description page of Project Settings.

#include "MinionMage.h"
#include "MinionMageAIController.h"

AMinionMage::AMinionMage()
{
	PrimaryActorTick.bCanEverTick = true;
	AIControllerClass = AMinionMageAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AMinionMage::BeginPlay()
{
	Super::BeginPlay();
}
