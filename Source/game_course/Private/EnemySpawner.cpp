// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemySpawner.h"
#include "MinionWarrior.h"
#include "MinionMage.h"
#include "MinionBrute.h"
#include "HealthBarWidget.h"
#include "BaseAttributeSet.h"
#include "HealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"

int32 AEnemySpawner::TotalSpawnCount  = 0;
int32 AEnemySpawner::LiveWarriorCount = 0;
int32 AEnemySpawner::LiveMageCount    = 0;
int32 AEnemySpawner::LiveBruteCount   = 0;

void AEnemySpawner::ResetCounters()
{
	TotalSpawnCount  = 0;
	LiveWarriorCount = 0;
	LiveMageCount    = 0;
	LiveBruteCount   = 0;
}

AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = true;

	// Capsule acts as root and as the ECC_Pawn collision shape so overlap queries find the spawner
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	CapsuleComponent->SetCapsuleHalfHeight(88.f);
	CapsuleComponent->SetCapsuleRadius(34.f);
	CapsuleComponent->SetCollisionObjectType(ECC_Pawn);
	CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CapsuleComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
	RootComponent = CapsuleComponent;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UBaseAttributeSet>(TEXT("AttributeSet"));

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	HealthBarWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidgetComponent"));
	HealthBarWidgetComponent->SetupAttachment(RootComponent);
	HealthBarWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarWidgetComponent->SetDrawSize(FVector2D(200.f, 50.f));
	HealthBarWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 150.f));
}

UAbilitySystemComponent* AEnemySpawner::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	// Init GAS
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	if (AttributeSet)
	{
		AttributeSet->InitMaxHealth(100.f);
		AttributeSet->InitHealth(100.f);
	}
	HealthComponent->BindToASC(AbilitySystemComponent);
	HealthComponent->OnHealthChanged.AddDynamic(this, &AEnemySpawner::OnHealthChanged);

	// Screen-space widget on a non-Pawn actor needs an explicit owning player
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		HealthBarWidgetComponent->SetOwnerPlayer(PC->GetLocalPlayer());
	}

	if (HealthBarWidgetClass)
	{
		HealthBarWidgetComponent->SetWidgetClass(HealthBarWidgetClass);
		HealthBarWidgetComponent->InitWidget();
	}
	if (UHealthBarWidget* W = Cast<UHealthBarWidget>(HealthBarWidgetComponent->GetWidget()))
	{
		W->SetHealthComponent(HealthComponent);
	}

	AddActorLocalRotation(FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f));

	if (AppearEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(AppearEffect, GetRootComponent());
	}
	if (AppearSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, AppearSound, GetActorLocation(), 0.25f);
	}

	GetWorldTimerManager().SetTimer(
		SpawnTimerHandle, this, &AEnemySpawner::TrySpawn, SpawnInterval, true);
}

void AEnemySpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	AddActorLocalRotation(FRotator(0.f, SpinSpeed * DeltaTime, 0.f));

	// Screen-space widgets on plain AActors can lose track of the component's
	// world position.  Explicitly keep the bar above the spawner each frame.
	if (HealthBarWidgetComponent)
	{
		HealthBarWidgetComponent->SetWorldLocation(GetActorLocation() + FVector(0.f, 0.f, 150.f));
	}
}

void AEnemySpawner::OnHealthChanged(float NewValue, float OldValue, float MaxValue)
{
	if (NewValue > 0.f) return;

	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);

	if (HealthBarWidgetComponent)
	{
		HealthBarWidgetComponent->SetVisibility(false);
	}
	if (DeathSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation(), 0.25f);
	}

	Destroy();
}

void AEnemySpawner::TrySpawn()
{
	const int32 Warriors = LiveWarriorCount;
	const int32 Mages    = LiveMageCount;
	const int32 Brutes   = LiveBruteCount;

	FVector SpawnOffset(FMath::FRandRange(-80.f, 80.f), FMath::FRandRange(-80.f, 80.f), 0.f);
	FTransform SpawnTransform(FRotator(0.f, GetActorRotation().Yaw, 0.f), GetActorLocation() + SpawnOffset);

	bool bSpawned = false;

	const bool bSpawnBrute = (TotalSpawnCount % 10 == 9) && BruteClass && Brutes < MaxBrutes;

	if (bSpawnBrute)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		GetWorld()->SpawnActor<AMinionBrute>(BruteClass, SpawnTransform, Params);
		bSpawned = true;
		TotalSpawnCount++;
		LiveBruteCount++;
	}
	else if (WarriorClass && Warriors < MaxWarriors)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		GetWorld()->SpawnActor<AMinionWarrior>(WarriorClass, SpawnTransform, Params);
		bSpawned = true;
		TotalSpawnCount++;
		LiveWarriorCount++;
	}

	if (bSpawned && SpawnSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SpawnSound, GetActorLocation(), 0.25f);
	}

	// Defer mage spawn to next frame to avoid 2 enemy spawns in one frame
	if (MageClass && Mages < MaxMages && Mages * 2 < Warriors)
	{
		FTimerHandle MageDeferHandle;
		GetWorldTimerManager().SetTimer(MageDeferHandle, [this, SpawnTransform]()
		{
			if (!MageClass) return;
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			GetWorld()->SpawnActor<AMinionMage>(MageClass, SpawnTransform, Params);
			LiveMageCount++;
			if (SpawnSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, SpawnSound, GetActorLocation(), 0.25f);
			}
		}, 0.05f, false);
	}
}
