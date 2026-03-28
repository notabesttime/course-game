// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseEnemy.h"
#include "MinionMage.generated.h"

UCLASS()
class GAME_COURSE_API AMinionMage : public ABaseEnemy
{
	GENERATED_BODY()

public:
	AMinionMage();

protected:
	virtual void BeginPlay() override;
};
