// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacter.h"
#include "PlayerMeleeAbility.h"
#include "PlayerRangedAbility.h"
#include "BaseEnemy.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "Framework/Application/SlateApplication.h"
#include "Engine/EngineTypes.h"
#include "HealthComponent.h"
#include "ManaComponent.h"
#include "CameraOcclusionComponent.h"
#include "PlayerShieldComponent.h"
#include "BaseAttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "CourseGameMode.h"
#include "GameFramework/WorldSettings.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	CameraOcclusionComponent = CreateDefaultSubobject<UCameraOcclusionComponent>(TEXT("CameraOcclusionComponent"));
	ShieldComponent = CreateDefaultSubobject<UPlayerShieldComponent>(TEXT("ShieldComponent"));

	HoverObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		if (MeleeAbilityClass)
		{
			MeleeAbilityHandle = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(MeleeAbilityClass, 1));
		}
		if (RangedAbilityClass)
		{
			RangedAbilityHandle = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(RangedAbilityClass, 1));
		}
	}

	if (HealthComponent)
	{
		HealthComponent->OnHealthChanged.AddDynamic(this, &APlayerCharacter::OnHealthChanged);
	}
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateHoveredEnemy();

	// Time slow mana drain: 30 per second (DeltaTime is unscaled for player due to counter-dilation)
	if (bTimeSlowActive && AbilitySystemComponent && AttributeSet)
	{
		float NewMana = AttributeSet->GetMana() - 30.f * DeltaTime;
		if (NewMana <= 0.f)
		{
			AttributeSet->SetMana(0.f);
			DeactivateTimeSlow();
		}
		else
		{
			AttributeSet->SetMana(NewMana);
		}
	}

	// Mana regeneration: 20 per second (suppressed while time slow is draining mana)
	if (!bTimeSlowActive && AbilitySystemComponent && AttributeSet)
	{
		const float CurrentMana = AttributeSet->GetMana();
		const float MaxMana     = AttributeSet->GetMaxMana();
		if (CurrentMana < MaxMana)
		{
			AttributeSet->SetMana(FMath::Min(CurrentMana + 20.f * DeltaTime, MaxMana));
		}
	}
}

void APlayerCharacter::UpdateHoveredEnemy()
{
	// Throttle to 20Hz — hover detection doesn't need sub-frame precision
	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastHoverUpdateTime < 0.05f) return;
	LastHoverUpdateTime = Now;

	ABaseEnemy* NewHovered = nullptr;

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		FHitResult HitResult;
		if (PC->GetHitResultUnderCursorForObjects(HoverObjectTypes, false, HitResult))
		{
			NewHovered = Cast<ABaseEnemy>(HitResult.GetActor());
		}
	}

	if (NewHovered != HoveredEnemy)
	{
		if (HoveredEnemy)
		{
			HoveredEnemy->StopHighlight();
		}
		HoveredEnemy = NewHovered;
		if (HoveredEnemy)
		{
			HoveredEnemy->StartHighlight();
		}
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MeleeInputAction)
		{
			EIC->BindAction(MeleeInputAction, ETriggerEvent::Started, this, &APlayerCharacter::ActivateMelee);
		}
		if (RangedInputAction)
		{
			EIC->BindAction(RangedInputAction, ETriggerEvent::Started, this, &APlayerCharacter::ActivateRanged);
		}
		if (MoveInputAction)
		{
			EIC->BindAction(MoveInputAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		}
		if (DebugDieInputAction)
		{
			EIC->BindAction(DebugDieInputAction, ETriggerEvent::Started, this, &APlayerCharacter::DebugDie);
		}
		if (RestartInputAction)
		{
			EIC->BindAction(RestartInputAction, ETriggerEvent::Started, this, &APlayerCharacter::QuickRestart);
		}
		if (DebugShieldInputAction)
		{
			EIC->BindAction(DebugShieldInputAction, ETriggerEvent::Started, this, &APlayerCharacter::DebugActivateShield);
		}
		if (TimeSlowInputAction)
		{
			EIC->BindAction(TimeSlowInputAction, ETriggerEvent::Started,   this, &APlayerCharacter::ActivateTimeSlow);
			EIC->BindAction(TimeSlowInputAction, ETriggerEvent::Completed, this, &APlayerCharacter::DeactivateTimeSlow);
		}
	}
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (Axis.IsNearlyZero()) return;

	// Use camera yaw so WASD aligns with what's on screen regardless of camera angle
	float CameraYaw = GetControlRotation().Yaw;
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		FVector CamLoc;
		FRotator CamRot;
		PC->GetPlayerViewPoint(CamLoc, CamRot);
		CameraYaw = CamRot.Yaw;
	}

	const FRotator YawRotation(0.f, CameraYaw, 0.f);
	const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDir   = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDir, Axis.Y);
	AddMovementInput(RightDir,   Axis.X);
}

void APlayerCharacter::ActivateMelee()
{
	if (HoveredEnemy && AbilitySystemComponent)
	{
		AbilitySystemComponent->TryActivateAbility(MeleeAbilityHandle);
		LastAttackTime = GetWorld()->GetTimeSeconds();
		if (MeleeAttackSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, MeleeAttackSound, GetActorLocation());
		}
	}
}

void APlayerCharacter::ActivateRanged()
{
	if (!AbilitySystemComponent) return;

	// Check and consume 10 mana
	if (AttributeSet)
	{
		if (AttributeSet->GetMana() < 10.f) return;
		AttributeSet->SetMana(AttributeSet->GetMana() - 10.f);
	}

	AbilitySystemComponent->TryActivateAbility(RangedAbilityHandle);
	LastAttackTime = GetWorld()->GetTimeSeconds();
	if (RangedAttackSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, RangedAttackSound, GetActorLocation());
	}
}

void APlayerCharacter::DebugDie()
{
	if (AttributeSet)
	{
		// Force health to 0, which triggers OnHealthChanged → full death flow
		if (AbilitySystemComponent)
		{
			AbilitySystemComponent->SetNumericAttributeBase(
				UBaseAttributeSet::GetHealthAttribute(), 0.f);
		}
	}
}

void APlayerCharacter::QuickRestart()
{
	if (ACourseGameMode* GM = Cast<ACourseGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		GM->SaveScoreOnly();
	}

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetShowMouseCursor(true);
		FInputModeGameAndUI Mode;
		Mode.SetHideCursorDuringCapture(false);
		PC->SetInputMode(Mode);
	}

	UGameplayStatics::OpenLevel(GetWorld(), FName(*GetWorld()->GetMapName()));
}

void APlayerCharacter::DebugActivateShield()
{
	UE_LOG(LogTemp, Warning, TEXT("DebugActivateShield called"));
	if (ShieldComponent)
	{
		ShieldComponent->ActivateShield(60.f);
		UE_LOG(LogTemp, Warning, TEXT("Shield activated"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ShieldComponent is null"));
	}
}

void APlayerCharacter::ActivateTimeSlow()
{
	if (bTimeSlowActive) return;
	if (!AttributeSet || AttributeSet->GetMana() <= 0.f) return;

	bTimeSlowActive = true;

	// Slow the whole world to 20%, counter-dilate the player to stay at 1x real speed
	GetWorld()->GetWorldSettings()->SetTimeDilation(0.2f);
	CustomTimeDilation = 5.0f;
}

void APlayerCharacter::DeactivateTimeSlow()
{
	if (!bTimeSlowActive) return;
	bTimeSlowActive = false;

	GetWorld()->GetWorldSettings()->SetTimeDilation(1.0f);
	CustomTimeDilation = 1.0f;
}

void APlayerCharacter::OnHealthChanged(float NewValue, float OldValue, float MaxValue)
{
	// Shield blocks all damage — restore health back to pre-hit value
	if (ShieldComponent && ShieldComponent->IsShieldActive() && NewValue < OldValue)
	{
		if (AbilitySystemComponent && AttributeSet)
		{
			AbilitySystemComponent->SetNumericAttributeBase(
				UBaseAttributeSet::GetHealthAttribute(), OldValue);
		}
		return;
	}

	if (NewValue <= 0.0f)
	{
		DeactivateTimeSlow();

		if (DeathSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
		}

		if (ACourseGameMode* GM = Cast<ACourseGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
		{
			GM->OnPlayerDied();
		}

		// Disable input and hide player
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			DisableInput(PC);
		}
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
	}
}
