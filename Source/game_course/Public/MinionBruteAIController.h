// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MinionBruteAIController.generated.h"

UCLASS()
class GAME_COURSE_API AMinionBruteAIController : public AAIController
{
	GENERATED_BODY()

public:
	AMinionBruteAIController();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	// How close the brute gets before stopping (melee range)
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float MeleeRange = 180.0f;

	// How often (seconds) to re-issue the move command
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float MoveUpdateInterval = 0.5f;

private:
	void UpdateMovement();

	FTimerHandle MoveTimerHandle;
};
