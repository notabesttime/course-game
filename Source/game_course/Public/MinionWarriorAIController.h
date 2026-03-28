// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MinionWarriorAIController.generated.h"

UCLASS()
class GAME_COURSE_API AMinionWarriorAIController : public AAIController
{
	GENERATED_BODY()

public:
	AMinionWarriorAIController();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	// How close the warrior gets before stopping (melee range)
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float MeleeRange = 150.0f;

	// How often (seconds) to re-issue the move command
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float MoveUpdateInterval = 0.5f;

private:
	void UpdateMovement();

	FTimerHandle MoveTimerHandle;
};
