// Fill out your copyright notice in the Description page of Project Settings.

#include "game_course.h"
#include "HealingOrb.h"
#include "MinionMageProjectile.h"
#include "PlayerAbilityProjectile.h"
#include "HAL/IConsoleManager.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, game_course, "game_course" );

namespace
{
	static FAutoConsoleCommand CmdPoolStats(
		TEXT("game.PoolStats"),
		TEXT("Log pooled actor stats (HealingOrb, MageProjectile, PlayerProjectile)."),
		FConsoleCommandDelegate::CreateLambda([]()
		{
			AHealingOrb::LogPoolStats();
			AMinionMageProjectile::LogPoolStats();
			APlayerAbilityProjectile::LogPoolStats();
		})
	);
}
