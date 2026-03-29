// Fill out your copyright notice in the Description page of Project Settings.

#include "MinionWarriorAIController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Navigation/PathFollowingComponent.h"

AMinionWarriorAIController::AMinionWarriorAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMinionWarriorAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	GetWorldTimerManager().SetTimer(
		MoveTimerHandle,
		this,
		&AMinionWarriorAIController::UpdateMovement,
		MoveUpdateInterval,
		true,
		FMath::FRandRange(0.1f, MoveUpdateInterval)  // stagger to avoid all-at-once pathfinding
	);
}

void AMinionWarriorAIController::OnUnPossess()
{
	Super::OnUnPossess();
	GetWorldTimerManager().ClearTimer(MoveTimerHandle);
}

void AMinionWarriorAIController::UpdateMovement()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("MinionWarriorAIController: Player pawn not found"));
		return;
	}

	EPathFollowingRequestResult::Type Result = MoveToActor(PlayerPawn, 20.f);
	if (Result == EPathFollowingRequestResult::Failed)
	{
		UE_LOG(LogTemp, Warning, TEXT("MinionWarriorAIController: MoveToActor FAILED — check NavMesh covers the warrior's location"));
	}
}
