// Fill out your copyright notice in the Description page of Project Settings.

#include "GameTimerWidget.h"
#include "WaveAnnouncementWidget.h"
#include "PlayerCharacter.h"
#include "PlayerShieldComponent.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void UGameTimerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (TimerText)
	{
		TimerText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		TimerText->SetRenderScale(FVector2D(1.f, 1.f));
	}

	// ShieldLabel is bound via BindWidgetOptional — configure it if present
	if (ShieldLabel)
	{
		FSlateFontInfo Font = ShieldLabel->GetFont();
		Font.Size = 18;
		ShieldLabel->SetFont(Font);
		ShieldLabel->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.5f, 0.f, 1.f)));
		ShieldLabel->SetJustification(ETextJustify::Center);
		ShieldLabel->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UGameTimerWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bStopped) ElapsedTime += InDeltaTime;

	// Shield label
	if (ShieldLabel)
	{
		APlayerCharacter* Player = Cast<APlayerCharacter>(
			UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
		UPlayerShieldComponent* Shield = Player
			? Player->GetShieldComponent() : nullptr;

		if (Shield && Shield->IsShieldActive())
		{
			float Remaining = Shield->GetShieldTimeRemaining();
			ShieldLabel->SetText(FText::FromString(
				FString::Printf(TEXT("Shield: %.1fs"), Remaining)));
			ShieldLabel->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			ShieldLabel->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

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
