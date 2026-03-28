// Fill out your copyright notice in the Description page of Project Settings.

#include "GameHUD.h"
#include "HealthBarWidget.h"
#include "GameTimerWidget.h"
#include "SpawnerIndicatorWidget.h"
#include "EnemySpawner.h"
#include "BaseCharacter.h"
#include "HealthComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanelSlot.h"
#include "EngineUtils.h"

void AGameHUD::BeginPlay()
{
	Super::BeginPlay();

	if (HealthBarClass)
	{
		HealthBarWidget = CreateWidget<UHealthBarWidget>(GetWorld(), HealthBarClass);
		if (HealthBarWidget)
		{
			HealthBarWidget->AddToViewport();
		}
	}

	if (TimerWidgetClass)
	{
		TimerWidget = CreateWidget<UGameTimerWidget>(GetWorld(), TimerWidgetClass);
		if (TimerWidget)
		{
			TimerWidget->AddToViewport(1);

			if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(TimerWidget->Slot))
			{
				Slot->SetAnchors(FAnchors(0.5f, 0.f, 0.5f, 0.f));
				Slot->SetAlignment(FVector2D(0.5f, 0.f));
				Slot->SetPosition(FVector2D(0.f, 20.f));
				Slot->SetAutoSize(true);
			}
		}
	}

	// Defer one tick so the player character's BeginPlay (and BindToASC) completes first
	GetWorldTimerManager().SetTimerForNextTick(this, &AGameHUD::BindPlayerHealth);
}

void AGameHUD::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateSpawnerIndicators();
}

void AGameHUD::BindPlayerHealth()
{
	if (!HealthBarWidget)
	{
		return;
	}

	APawn* PlayerPawn = GetOwningPawn();
	if (PlayerPawn)
	{
		if (UHealthComponent* HealthComp = PlayerPawn->FindComponentByClass<UHealthComponent>())
		{
			HealthBarWidget->SetHealthComponent(HealthComp);
		}
	}
}

void AGameHUD::UpdateSpawnerIndicators()
{
	if (!SpawnerIndicatorClass)
	{
		return;
	}

	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}

	// Remove indicators for destroyed spawners
	for (auto It = SpawnerIndicators.CreateIterator(); It; ++It)
	{
		if (!IsValid(It->Key))
		{
			if (It->Value)
			{
				It->Value->RemoveFromParent();
			}
			It.RemoveCurrent();
		}
	}

	// Create indicators for new spawners, update positions for all
	for (TActorIterator<AEnemySpawner> It(GetWorld()); It; ++It)
	{
		AEnemySpawner* Spawner = *It;

		if (!SpawnerIndicators.Contains(Spawner))
		{
			USpawnerIndicatorWidget* Indicator = CreateWidget<USpawnerIndicatorWidget>(PC, SpawnerIndicatorClass);
			if (Indicator)
			{
				Indicator->AddToViewport(2);
				if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(Indicator->Slot))
				{
					Slot->SetAlignment(FVector2D(0.5f, 0.5f));
				}
				SpawnerIndicators.Add(Spawner, Indicator);
			}
		}

		USpawnerIndicatorWidget* Indicator = SpawnerIndicators[Spawner];
		if (!Indicator)
		{
			continue;
		}

		int32 ViewW, ViewH;
		PC->GetViewportSize(ViewW, ViewH);
		FVector2D ViewportSize(ViewW, ViewH);
		FVector2D Center = ViewportSize * 0.5f;

		// Project spawner base location (used for rotation target)
		FVector2D SpawnerScreenPos;
		PC->ProjectWorldLocationToScreen(Spawner->GetActorLocation(), SpawnerScreenPos);

		// Project offset location (used for on-screen icon placement)
		FVector2D OffsetScreenPos;
		PC->ProjectWorldLocationToScreen(
			Spawner->GetActorLocation() + FVector(0.f, 0.f, IndicatorHeightOffset),
			OffsetScreenPos);

		// Detect if behind camera
		FVector CamLoc;
		FRotator CamRot;
		PC->GetPlayerViewPoint(CamLoc, CamRot);
		FVector ToSpawner = (Spawner->GetActorLocation() - CamLoc).GetSafeNormal();
		bool bBehind = FVector::DotProduct(CamRot.Vector(), ToSpawner) < 0.f;

		bool bOnScreen = !bBehind
			&& OffsetScreenPos.X >= IndicatorEdgeMargin && OffsetScreenPos.X <= ViewW - IndicatorEdgeMargin
			&& OffsetScreenPos.Y >= IndicatorEdgeMargin && OffsetScreenPos.Y <= ViewH - IndicatorEdgeMargin;

		FVector2D FinalPos;

		if (bOnScreen)
		{
			FinalPos = OffsetScreenPos;
		}
		else
		{
			if (bBehind)
			{
				OffsetScreenPos = 2.f * Center - OffsetScreenPos;
			}
			FinalPos = ClampToScreenEdge(OffsetScreenPos, ViewportSize);
		}

		Indicator->SetPositionInViewport(FinalPos + FVector2D(IndicatorHorizontalOffset, 0.f));
	}
}

FVector2D AGameHUD::ClampToScreenEdge(FVector2D ScreenPos, FVector2D ViewportSize) const
{
	FVector2D Center = ViewportSize * 0.5f;
	FVector2D Dir = ScreenPos - Center;

	if (Dir.IsNearlyZero())
	{
		Dir = FVector2D(1.f, 0.f);
	}
	Dir.Normalize();

	float HalfW = Center.X - IndicatorEdgeMargin;
	float HalfH = Center.Y - IndicatorEdgeMargin;

	float ScaleX = FMath::Abs(Dir.X) > KINDA_SMALL_NUMBER ? HalfW / FMath::Abs(Dir.X) : FLT_MAX;
	float ScaleY = FMath::Abs(Dir.Y) > KINDA_SMALL_NUMBER ? HalfH / FMath::Abs(Dir.Y) : FLT_MAX;

	return Center + Dir * FMath::Min(ScaleX, ScaleY);
}
