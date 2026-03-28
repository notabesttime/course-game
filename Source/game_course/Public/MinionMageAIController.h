// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MinionMageAIController.generated.h"

UCLASS()
class GAME_COURSE_API AMinionMageAIController : public AAIController
{
	GENERATED_BODY()

public:
	AMinionMageAIController();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	// Distance the mage tries to maintain from the player
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float DesiredRange = 600.0f;

	// How close to the target position before stopping (avoids jitter)
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float AcceptanceRadius = 75.0f;

	// How often to recalculate position
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float MoveUpdateInterval = 0.5f;

	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

private:
	void UpdateMovement();
	void FacePlayer();

	FTimerHandle MoveTimerHandle;
};
