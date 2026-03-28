// Fill out your copyright notice in the Description page of Project Settings.

#include "SpawnerIndicatorWidget.h"

FVector2D USpawnerIndicatorWidget::GetIconSize() const
{
	// Use the widget's actual rendered size from the previous frame — this is the
	// size that matters for screen-edge clamping. GetDesiredSize() on a stretched
	// UImage can return the full viewport size and break the margin calculation.
	FVector2D CachedSize = GetCachedGeometry().GetLocalSize();
	if (CachedSize.X > 1.f && CachedSize.Y > 1.f)
	{
		return CachedSize;
	}
	return FVector2D(64.f, 64.f);
}
