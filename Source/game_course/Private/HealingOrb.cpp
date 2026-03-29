// Fill out your copyright notice in the Description page of Project Settings.

#include "HealingOrb.h"
#include "PlayerCharacter.h"
#include "BaseAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

AHealingOrb::AHealingOrb()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetSphereRadius(40.f);
	CollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	RootComponent = CollisionSphere;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AHealingOrb::BeginPlay()
{
	Super::BeginPlay();

	CachedPlayer = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
}

void AHealingOrb::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!CachedPlayer || !IsValid(CachedPlayer))
	{
		return;
	}

	// Stay still for AttackPauseTime after the player's last attack
	float TimeSinceAttack = GetWorld()->GetTimeSeconds() - CachedPlayer->GetLastAttackTime();
	if (TimeSinceAttack < AttackPauseTime)
	{
		return;
	}

	FVector ToPlayer = CachedPlayer->GetActorLocation() - GetActorLocation();
	float Dist = ToPlayer.Size();

	if (Dist <= PickupRadius)
	{
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(CachedPlayer))
		{
			UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
			if (ASC)
			{
				const UBaseAttributeSet* AttrSet = ASC->GetSet<UBaseAttributeSet>();
				if (AttrSet)
				{
					float NewHealth = FMath::Min(AttrSet->GetMaxHealth(), AttrSet->GetHealth() + HealAmount);
					ASC->SetNumericAttributeBase(UBaseAttributeSet::GetHealthAttribute(), NewHealth);

					float NewMana = FMath::Min(AttrSet->GetMaxMana(), AttrSet->GetMana() + 10.f);
					ASC->SetNumericAttributeBase(UBaseAttributeSet::GetManaAttribute(), NewMana);
				}
			}
		}
		Destroy();
		return;
	}

	AddActorWorldOffset(ToPlayer.GetSafeNormal() * MoveSpeed * DeltaTime);
}
