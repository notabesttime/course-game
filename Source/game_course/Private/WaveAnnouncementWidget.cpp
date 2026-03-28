// Fill out your copyright notice in the Description page of Project Settings.

#include "WaveAnnouncementWidget.h"
#include "Components/TextBlock.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Blueprint/WidgetTree.h"

void UWaveAnnouncementWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Root"));
	WidgetTree->RootWidget = Root;

	WaveLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("WaveLabel"));

	FSlateFontInfo Font = WaveLabel->GetFont();
	Font.Size = FontSize;
	WaveLabel->SetFont(Font);
	WaveLabel->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	WaveLabel->SetJustification(ETextJustify::Center);

	UOverlaySlot* OverlaySlot = Root->AddChildToOverlay(WaveLabel);
	OverlaySlot->SetHorizontalAlignment(HAlign_Center);
	OverlaySlot->SetVerticalAlignment(VAlign_Center);
}

void UWaveAnnouncementWidget::Show(int32 WaveNumber)
{
	if (!WaveLabel)
	{
		return;
	}

	WaveLabel->SetText(FText::FromString(FString::Printf(TEXT("Wave %d"), WaveNumber)));
	WaveLabel->SetRenderScale(FVector2D(StartScale, StartScale));
	WaveLabel->SetRenderOpacity(1.f);

	TimeRemaining = AnimDuration;
	bAnimating = true;

	AddToViewport(10);
}

void UWaveAnnouncementWidget::TickAnimation(float DeltaTime)
{
	if (!bAnimating || !WaveLabel)
	{
		return;
	}

	TimeRemaining -= DeltaTime;

	if (TimeRemaining <= 0.f)
	{
		bAnimating = false;
		RemoveFromParent();
		return;
	}

	float T = 1.f - (TimeRemaining / AnimDuration);

	float Scale = FMath::Lerp(StartScale, EndScale, T);
	WaveLabel->SetRenderScale(FVector2D(Scale, Scale));
	WaveLabel->SetRenderOpacity(1.f - T);
}
