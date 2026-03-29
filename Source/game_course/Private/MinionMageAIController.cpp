// Fill out your copyright notice in the Description page of Project Settings.

#include "MinionMageAIController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AMinionMageAIController::AMinionMageAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMinionMageAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	GetWorldTimerManager().SetTimer(
		MoveTimerHandle,
		this,
		&AMinionMageAIController::UpdateMovement,
		MoveUpdateInterval,
		true,
		FMath::FRandRange(0.1f, MoveUpdateInterval)  // stagger to avoid all-at-once pathfinding
	);
}

void AMinionMageAIController::OnUnPossess()
{
	Super::OnUnPossess();
	GetWorldTimerManager().ClearTimer(MoveTimerHandle);
}

void AMinionMageAIController::UpdateMovement()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	APawn* MyPawn = GetPawn();
	if (!PlayerPawn || !MyPawn)
	{
		return;
	}

	// Direction pointing from player toward the mage (away from player)
	FVector ToMage = (MyPawn->GetActorLocation() - PlayerPawn->GetActorLocation());
	float CurrentDistance = ToMage.Size();

	// If already within acceptance radius of the desired range, do nothing
	if (FMath::Abs(CurrentDistance - DesiredRange) <= AcceptanceRadius)
	{
		StopMovement();
		FacePlayer();
		return;
	}

	// Target position is DesiredRange units away from the player, along the mage-player axis
	FVector Direction = ToMage.GetSafeNormal();
	// Fallback if mage is on top of the player
	if (Direction.IsNearlyZero())
	{
		Direction = MyPawn->GetActorForwardVector();
	}

	FVector TargetLocation = PlayerPawn->GetActorLocation() + Direction * DesiredRange;
	MoveToLocation(TargetLocation, AcceptanceRadius);
}

void AMinionMageAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);
	FacePlayer();
}

void AMinionMageAIController::FacePlayer()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	APawn* MyPawn = GetPawn();
	if (!PlayerPawn || !MyPawn)
	{
		return;
	}

	FVector Direction = (PlayerPawn->GetActorLocation() - MyPawn->GetActorLocation()).GetSafeNormal();
	FRotator LookRotation = FRotationMatrix::MakeFromX(Direction).Rotator();
	MyPawn->SetActorRotation(FRotator(0.f, LookRotation.Yaw, 0.f));
}
