// Fill out your copyright notice in the Description page of Project Settings.

#include "GameTimerWidget.h"
#include "WaveAnnouncementWidget.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"

void UGameTimerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (TimerText)
	{
		TimerText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		TimerText->SetRenderScale(FVector2D(1.f, 1.f));
	}
}

void UGameTimerWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bStopped) ElapsedTime += InDeltaTime;

	if (ActiveAnnouncement && ActiveAnnouncement->IsAnimating())
	{
		ActiveAnnouncement->TickAnimation(InDeltaTime);
	}

	if (TimerText)
	{
		TimerText->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), ElapsedTime)));

		if (bFlashing)
		{
			FlashTimeRemaining -= InDeltaTime;

			if (FlashTimeRemaining <= 0.f)
			{
				bFlashing = false;
				TimerText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
				TimerText->SetRenderScale(FVector2D(1.f, 1.f));
			}
			else
			{
				float T = 1.f - (FlashTimeRemaining / FlashDuration);
				float Wave = FMath::Abs(FMath::Sin(T * PI * PulseCount));

				float Scale = FMath::Lerp(1.f, MaxPulseScale, Wave);
				TimerText->SetRenderScale(FVector2D(Scale, Scale));

				FLinearColor Color = FLinearColor::LerpUsingHSV(FLinearColor::White, FLinearColor::Red, Wave);
				TimerText->SetColorAndOpacity(FSlateColor(Color));
			}
		}
	}
}

void UGameTimerWidget::StartFlash(int32 WaveNumber)
{
	bFlashing = true;
	FlashTimeRemaining = FlashDuration;

	ActiveAnnouncement = CreateWidget<UWaveAnnouncementWidget>(
		GetOwningPlayer(),
		UWaveAnnouncementWidget::StaticClass()
	);
	if (ActiveAnnouncement)
	{
		ActiveAnnouncement->Show(WaveNumber);
	}
}
