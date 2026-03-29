// Fill out your copyright notice in the Description page of Project Settings.

#include "MinionBruteAIController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Navigation/PathFollowingComponent.h"

AMinionBruteAIController::AMinionBruteAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMinionBruteAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	GetWorldTimerManager().SetTimer(
		MoveTimerHandle,
		this,
		&AMinionBruteAIController::UpdateMovement,
		MoveUpdateInterval,
		true,
		0.0f
	);
}

void AMinionBruteAIController::OnUnPossess()
{
	Super::OnUnPossess();
	GetWorldTimerManager().ClearTimer(MoveTimerHandle);
}

void AMinionBruteAIController::UpdateMovement()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn)
	{
		return;
	}

	MoveToActor(PlayerPawn, MeleeRange);
}
