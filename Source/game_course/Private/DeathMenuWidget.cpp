// Fill out your copyright notice in the Description page of Project Settings.

#include "DeathMenuWidget.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Blueprint/WidgetTree.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/PlayerController.h"

void UDeathMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// ---- Overlay root: black background + centered content ----
	UOverlay* OverlayRoot = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
	WidgetTree->RootWidget = OverlayRoot;

	// Black semi-transparent background filling the whole screen
	UBorder* Background = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	Background->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.82f));
	if (UOverlaySlot* BgSlot = OverlayRoot->AddChildToOverlay(Background))
	{
		BgSlot->SetHorizontalAlignment(HAlign_Fill);
		BgSlot->SetVerticalAlignment(VAlign_Fill);
	}

	// Content box centered on screen
	RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	if (UOverlaySlot* ContentSlot = OverlayRoot->AddChildToOverlay(RootBox))
	{
		ContentSlot->SetHorizontalAlignment(HAlign_Center);
		ContentSlot->SetVerticalAlignment(VAlign_Center);
	}

	auto AddText = [&](const FString& Str, int32 Size, FLinearColor Color) -> UTextBlock*
	{
		UTextBlock* TB = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		TB->SetText(FText::FromString(Str));
		FSlateFontInfo Font = TB->GetFont();
		Font.Size = Size;
		TB->SetFont(Font);
		TB->SetColorAndOpacity(FSlateColor(Color));
		TB->SetJustification(ETextJustify::Center);

		UVerticalBoxSlot* Slot = RootBox->AddChildToVerticalBox(TB);
		Slot->SetHorizontalAlignment(HAlign_Center);
		Slot->SetPadding(FMargin(0.f, 8.f));
		return TB;
	};

	auto AddButton = [&](const FString& Label, FLinearColor BgColor, const FName& CallbackName)
	{
		UButton* Btn = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
		FButtonStyle Style = Btn->GetStyle();
		Style.Normal.TintColor    = FSlateColor(BgColor);
		Style.Hovered.TintColor   = FSlateColor(BgColor * 1.3f);
		Style.Pressed.TintColor   = FSlateColor(BgColor * 0.7f);
		Btn->SetStyle(Style);

		UTextBlock* BtnLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		BtnLabel->SetText(FText::FromString(Label));
		FSlateFontInfo Font = BtnLabel->GetFont();
		Font.Size = 22;
		BtnLabel->SetFont(Font);
		BtnLabel->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		BtnLabel->SetJustification(ETextJustify::Center);
		Btn->AddChild(BtnLabel);

		UVerticalBoxSlot* BtnSlot = RootBox->AddChildToVerticalBox(Btn);
		BtnSlot->SetHorizontalAlignment(HAlign_Center);
		BtnSlot->SetPadding(FMargin(0.f, 10.f));

		if (CallbackName == "Restart")
			Btn->OnClicked.AddDynamic(this, &UDeathMenuWidget::OnRestartClicked);
		else
			Btn->OnClicked.AddDynamic(this, &UDeathMenuWidget::OnExitClicked);
	};

	AddText(TEXT("YOU DIED"), 64, FLinearColor::Red);
	ScoreLabel    = AddText(TEXT("Survived: 0.0s"), 32, FLinearColor::White);
	Top5Label     = AddText(TEXT(""), 28, FLinearColor::Red);
	HighScoreList = AddText(TEXT(""), 22, FLinearColor(0.8f, 0.8f, 0.8f, 1.f));
	AddButton(TEXT("TRY AGAIN"), FLinearColor(0.1f, 0.4f, 0.1f, 1.f), "Restart");
	AddButton(TEXT("EXIT"),      FLinearColor(0.4f, 0.1f, 0.1f, 1.f), "Exit");

	// Small clear scores button
	UButton* ClearBtn = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
	FButtonStyle ClearStyle = ClearBtn->GetStyle();
	ClearStyle.Normal.TintColor  = FSlateColor(FLinearColor(0.2f, 0.2f, 0.2f, 1.f));
	ClearStyle.Hovered.TintColor = FSlateColor(FLinearColor(0.35f, 0.35f, 0.35f, 1.f));
	ClearStyle.Pressed.TintColor = FSlateColor(FLinearColor(0.1f, 0.1f, 0.1f, 1.f));
	ClearBtn->SetStyle(ClearStyle);
	ClearBtn->OnClicked.AddDynamic(this, &UDeathMenuWidget::OnClearScoresClicked);

	UTextBlock* ClearLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	ClearLabel->SetText(FText::FromString(TEXT("Clear Scores")));
	FSlateFontInfo ClearFont = ClearLabel->GetFont();
	ClearFont.Size = 14;
	ClearLabel->SetFont(ClearFont);
	ClearLabel->SetColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f, 1.f)));
	ClearLabel->SetJustification(ETextJustify::Center);
	ClearBtn->AddChild(ClearLabel);

	UVerticalBoxSlot* ClearSlot = RootBox->AddChildToVerticalBox(ClearBtn);
	ClearSlot->SetHorizontalAlignment(HAlign_Center);
	ClearSlot->SetPadding(FMargin(0.f, 4.f));
}

void UDeathMenuWidget::Show(float SurvivalTime, const TArray<float>& TopScores)
{
	// Current run score
	if (ScoreLabel)
	{
		ScoreLabel->SetText(FText::FromString(
			FString::Printf(TEXT("Survived: %.1f s"), SurvivalTime)));
	}

	// Top 5 badge — TopScores already contains the new score sorted in
	if (Top5Label)
	{
		bool bMadeTop5 = false;
		for (const float Score : TopScores)
		{
			if (FMath::IsNearlyEqual(Score, SurvivalTime, 0.05f))
			{
				bMadeTop5 = true;
				break;
			}
		}
		Top5Label->SetText(bMadeTop5
			? FText::FromString(TEXT("Made Top 5!"))
			: FText::GetEmpty());
	}

	// Top 5 list
	if (HighScoreList)
	{
		FString List = TEXT("--- TOP 5 ---\n");
		for (int32 i = 0; i < TopScores.Num(); i++)
		{
			// Highlight the current run's entry
			const bool bCurrent = FMath::IsNearlyEqual(TopScores[i], SurvivalTime, 0.05f);
			List += FString::Printf(TEXT("%d.  %.1f s%s\n"),
				i + 1, TopScores[i], bCurrent ? TEXT("  <") : TEXT(""));
		}
		HighScoreList->SetText(FText::FromString(List));
	}

	CachedTopScores = TopScores;

	AddToViewport(10);

	// Show cursor and stop game
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetShowMouseCursor(true);
		PC->SetInputMode(FInputModeUIOnly());
	}
}

void UDeathMenuWidget::OnRestartClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetShowMouseCursor(true);
		FInputModeGameAndUI Mode;
		Mode.SetHideCursorDuringCapture(false);
		PC->SetInputMode(Mode);
	}

	const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this, true);
	if (!CurrentLevelName.IsEmpty())
	{
		UGameplayStatics::OpenLevel(this, FName(*CurrentLevelName));
	}
}

void UDeathMenuWidget::OnClearScoresClicked()
{
	UGameplayStatics::DeleteGameInSlot(TEXT("HighScores"), 0);
	CachedTopScores.Empty();

	if (HighScoreList)
	{
		HighScoreList->SetText(FText::FromString(TEXT("--- TOP 5 ---\n")));
	}
	if (Top5Label)
	{
		Top5Label->SetText(FText::GetEmpty());
	}
}

void UDeathMenuWidget::OnExitClicked()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), GetOwningPlayer(),
		EQuitPreference::Quit, false);
}
