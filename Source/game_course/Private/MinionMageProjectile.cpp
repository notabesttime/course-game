// Fill out your copyright notice in the Description page of Project Settings.

#include "MinionMageProjectile.h"
#include "PlayerCharacter.h"
#include "BaseAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AMinionMageProjectile::AMinionMageProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetSphereRadius(20.f);
	SphereComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	RootComponent = SphereComponent;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 1200.f;
	ProjectileMovement->MaxSpeed = 1200.f;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->bRotationFollowsVelocity = true;

	InitialLifeSpan = 5.f;
}

void AMinionMageProjectile::BeginPlay()
{
	Super::BeginPlay();
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AMinionMageProjectile::OnOverlapBegin);
}

void AMinionMageProjectile::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (bHasImpacted || !OtherActor || OtherActor == GetOwner())
	{
		return;
	}

	APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
	if (!Player)
	{
		return;
	}

	bHasImpacted = true;

	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Player))
	{
		UAbilitySystemComponent* TargetASC = ASI->GetAbilitySystemComponent();
		if (TargetASC)
		{
			const UBaseAttributeSet* AttrSet = TargetASC->GetSet<UBaseAttributeSet>();
			if (AttrSet)
			{
				float NewHealth = FMath::Max(0.f, AttrSet->GetHealth() - DamageAmount);
				TargetASC->SetNumericAttributeBase(UBaseAttributeSet::GetHealthAttribute(), NewHealth);
			}
		}
	}

	OnProjectileImpact(GetActorLocation());
	Destroy();
}
