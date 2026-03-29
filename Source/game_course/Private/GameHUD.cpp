// Fill out your copyright notice in the Description page of Project Settings.

#include "GameHUD.h"
#include "HealthBarWidget.h"
#include "GameTimerWidget.h"
#include "SpawnerIndicatorWidget.h"
#include "GlassSphereHealthWidget.h"
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
	GetWorldTimerManager().SetTimerForNextTick(this, &AGameHUD::SetupGlassSphereHealth);
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

void AGameHUD::SetupGlassSphereHealth()
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	// Create directly from the C++ class — no Blueprint asset needed
	GlassSphereHealth = CreateWidget<UGlassSphereHealthWidget>(PC, UGlassSphereHealthWidget::StaticClass());
	if (!GlassSphereHealth) return;

	GlassSphereHealth->AddToViewport(3);

	// Bottom-left: alignment (0,1) means the widget's bottom-left corner sits at the position
	GlassSphereHealth->SetAlignmentInViewport(FVector2D(0.f, 1.f));

	int32 ViewW, ViewH;
	PC->GetViewportSize(ViewW, ViewH);
	GlassSphereHealth->SetPositionInViewport(FVector2D(20.f, (float)ViewH - 20.f));

	// Bind to player health
	APawn* PlayerPawn = PC->GetPawn();
	if (PlayerPawn)
	{
		if (UHealthComponent* HealthComp = PlayerPawn->FindComponentByClass<UHealthComponent>())
		{
			GlassSphereHealth->SetHealthComponent(HealthComp);
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
				Indicator->SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
				Spawner->OnDestroyed.AddDynamic(this, &AGameHUD::OnSpawnerDestroyed);
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

		// Project offset location (used for on-screen icon placement)
		FVector SpawnerOffsetLoc = Spawner->GetActorLocation() + FVector(0.f, 0.f, IndicatorHeightOffset);
		FVector2D OffsetScreenPos;
		PC->ProjectWorldLocationToScreen(SpawnerOffsetLoc, OffsetScreenPos);

		// Camera axes for stable off-screen direction
		FVector CamLoc;
		FRotator CamRot;
		PC->GetPlayerViewPoint(CamLoc, CamRot);
		FMatrix CamMatrix = FRotationMatrix(CamRot);
		FVector CamForward = CamMatrix.GetScaledAxis(EAxis::X);
		FVector CamRight   = CamMatrix.GetScaledAxis(EAxis::Y);
		FVector CamUp      = CamMatrix.GetScaledAxis(EAxis::Z);

		FVector ToTarget = (SpawnerOffsetLoc - CamLoc).GetSafeNormal();
		bool bBehind = FVector::DotProduct(CamForward, ToTarget) < 0.f;

		FVector2D IconSize = Indicator->GetIconSize();
		const float HalfIcon = FMath::Max(IconSize.X, IconSize.Y) * 0.5f;
		const float EffectiveMargin = IndicatorEdgeMargin + HalfIcon;

		// Use only the icon half-size as the on-screen threshold so the icon
		// follows the spawner as long as it fits within the viewport.
		// EffectiveMargin (which adds the edge padding) is only for clamped positions.
		bool bOnScreen = !bBehind
			&& OffsetScreenPos.X >= HalfIcon && OffsetScreenPos.X <= ViewW - HalfIcon
			&& OffsetScreenPos.Y >= HalfIcon && OffsetScreenPos.Y <= ViewH - HalfIcon;

		FVector2D FinalPos;

		if (bOnScreen)
		{
			FinalPos = OffsetScreenPos;
		}
		else
		{
			// Use view-space direction — continuous across the behind-camera boundary,
			// avoids the jump caused by flipping the projected position.
			float DirX = FVector::DotProduct(ToTarget, CamRight);
			float DirY = -FVector::DotProduct(ToTarget, CamUp); // screen Y is inverted
			// No negation for behind-camera: view-space direction is continuous across that boundary.
			// The indicator correctly points toward the spawner even when it's behind the camera.
			FVector2D Dir(DirX, DirY);
			if (Dir.IsNearlyZero()) Dir = FVector2D(1.f, 0.f);
			Dir.Normalize();
			FinalPos = ClampToScreenEdge(Dir, Center, EffectiveMargin);
		}

		Indicator->SetPositionInViewport(FinalPos + FVector2D(IndicatorHorizontalOffset, 0.f));
	}
}

void AGameHUD::OnSpawnerDestroyed(AActor* DestroyedActor)
{
	AEnemySpawner* Spawner = Cast<AEnemySpawner>(DestroyedActor);
	if (!Spawner)
	{
		return;
	}

	if (USpawnerIndicatorWidget** Found = SpawnerIndicators.Find(Spawner))
	{
		if (*Found)
		{
			(*Found)->RemoveFromParent();
		}
		SpawnerIndicators.Remove(Spawner);
	}
}

FVector2D AGameHUD::ClampToScreenEdge(FVector2D Dir, FVector2D Center, float EffectiveMargin) const
{
	float HalfW = Center.X - EffectiveMargin;
	float HalfH = Center.Y - EffectiveMargin;

	float ScaleX = FMath::Abs(Dir.X) > KINDA_SMALL_NUMBER ? HalfW / FMath::Abs(Dir.X) : FLT_MAX;
	float ScaleY = FMath::Abs(Dir.Y) > KINDA_SMALL_NUMBER ? HalfH / FMath::Abs(Dir.Y) : FLT_MAX;

	return Center + Dir * FMath::Min(ScaleX, ScaleY);
}
