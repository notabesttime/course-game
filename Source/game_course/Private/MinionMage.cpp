// Fill out your copyright notice in the Description page of Project Settings.

#include "MinionMage.h"
#include "MinionMageAIController.h"
#include "MinionMageProjectile.h"
#include "EnemySpawner.h"
#include "PlayerCharacter.h"
#include "PlayerShieldComponent.h"
#include "BaseEnemy.h"
#include "BaseAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Animation/AnimSequence.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/OverlapResult.h"

AMinionMage::AMinionMage()
{
	PrimaryActorTick.bCanEverTick = true;
	AIControllerClass = AMinionMageAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AMinionMage::BeginPlay()
{
	Super::BeginPlay();

	if (SkinVariants.Num() > 0)
	{
		USkeletalMesh* ChosenSkin = SkinVariants[FMath::RandRange(0, SkinVariants.Num() - 1)];
		if (ChosenSkin)
		{
			GetMesh()->SetSkeletalMesh(ChosenSkin);
		}
	}

	if (GetMesh()->GetSkeletalMeshAsset())
	{
		GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		SetAnimState(EMageAnimState::Idle);
	}

	CachedPlayer = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	// Heal search runs at 4Hz instead of every frame
	GetWorldTimerManager().SetTimer(
		HealSearchTimer, this, &AMinionMage::TryHealAlly, 0.25f, true);

	// Stagger first shot so mages spawned together don't all fire at once
	bAttackOnCooldown = true;
	GetWorldTimerManager().SetTimer(
		AttackCooldownTimer,
		[this]() { bAttackOnCooldown = false; },
		FMath::FRandRange(0.f, AttackCooldown),
		false
	);
}

void AMinionMage::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateAnimations();
	TryAttackPlayer();
	// TryHealAlly is called by HealSearchTimer at 4Hz
}

void AMinionMage::UpdateAnimations()
{
	if (CurrentAnimState == EMageAnimState::Casting)
	{
		return;
	}

	float Speed = GetCharacterMovement()->Velocity.Size();
	SetAnimState(Speed > 50.f ? EMageAnimState::Running : EMageAnimState::Idle);
}

void AMinionMage::SetAnimState(EMageAnimState NewState)
{
	if (CurrentAnimState == NewState)
	{
		return;
	}

	CurrentAnimState = NewState;

	switch (NewState)
	{
	case EMageAnimState::Idle:
		if (IdleAnimation)
		{
			GetMesh()->PlayAnimation(IdleAnimation, true);
		}
		break;
	case EMageAnimState::Running:
		if (RunAnimation)
		{
			GetMesh()->PlayAnimation(RunAnimation, true);
		}
		break;
	case EMageAnimState::Casting:
		if (CastAnimations.Num() > 0)
		{
			UAnimSequence* Anim = CastAnimations[FMath::RandRange(0, CastAnimations.Num() - 1)];
			if (Anim)
			{
				GetMesh()->PlayAnimation(Anim, false);
			}
		}
		break;
	}
}

void AMinionMage::TryAttackPlayer()
{
	if (bAttackOnCooldown || !ProjectileClass || !CachedPlayer)
	{
		return;
	}

	APlayerCharacter* Player = CachedPlayer;

	float DistSq = FVector::DistSquared(GetActorLocation(), Player->GetActorLocation());
	if (DistSq > AttackRange * AttackRange)
	{
		return;
	}

	// Face player before firing
	FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
	ToPlayer.Z = 0.f;
	if (!ToPlayer.IsNearlyZero())
	{
		SetActorRotation(ToPlayer.Rotation());
	}

	// Spawn projectile slightly in front of and above the mage
	FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 60.f + FVector(0.f, 0.f, 60.f);
	FVector Direction = (Player->GetActorLocation() + FVector(0.f, 0.f, 50.f) - SpawnLocation).GetSafeNormal();
	FRotator SpawnRotation = Direction.Rotation();

	FActorSpawnParameters Params;
	Params.Instigator = this;
	Params.Owner = this;
	GetWorld()->SpawnActor<AMinionMageProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, Params);

	SetAnimState(EMageAnimState::Casting);

	if (AttackSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, AttackSound, GetActorLocation(), 0.25f);
	}

	bAttackOnCooldown = true;
	float Jitter = FMath::FRandRange(0.f, AttackCooldown * 0.5f);
	GetWorldTimerManager().SetTimer(
		AttackCooldownTimer,
		[this]()
		{
			bAttackOnCooldown = false;
			CurrentAnimState = EMageAnimState::Idle;
		},
		AttackCooldown + Jitter,
		false
	);
}

void AMinionMage::OnDied()
{
	AEnemySpawner::LiveMageCount = FMath::Max(0, AEnemySpawner::LiveMageCount - 1);

	if (CachedPlayer)
	{
		if (UPlayerShieldComponent* Shield = CachedPlayer->GetShieldComponent())
		{
			Shield->ActivateShield();
		}
	}
}

void AMinionMage::TryHealAlly()
{
	if (bHealOnCooldown)
	{
		return;
	}

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		GetActorLocation(),
		FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn),
		FCollisionShape::MakeSphere(HealRange),
		Params
	);

	// Find the most damaged ally (lowest health %) that isn't full
	ABaseEnemy* BestTarget = nullptr;
	float LowestHealthPct = 1.0f;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		ABaseEnemy* Ally = Cast<ABaseEnemy>(Overlap.GetActor());
		if (!Ally || !IsValid(Ally))
		{
			continue;
		}

		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Ally))
		{
			UAbilitySystemComponent* AllyASC = ASI->GetAbilitySystemComponent();
			if (!AllyASC) continue;

			const UBaseAttributeSet* AttrSet = AllyASC->GetSet<UBaseAttributeSet>();
			if (!AttrSet || AttrSet->GetMaxHealth() <= 0.f) continue;

			float HealthPct = AttrSet->GetHealth() / AttrSet->GetMaxHealth();
			if (HealthPct < LowestHealthPct)
			{
				LowestHealthPct = HealthPct;
				BestTarget = Ally;
			}
		}
	}

	if (!BestTarget)
	{
		return;
	}

	// Apply heal
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(BestTarget))
	{
		UAbilitySystemComponent* AllyASC = ASI->GetAbilitySystemComponent();
		const UBaseAttributeSet* AttrSet = AllyASC->GetSet<UBaseAttributeSet>();
		float NewHealth = FMath::Min(AttrSet->GetMaxHealth(), AttrSet->GetHealth() + HealAmount);
		AllyASC->SetNumericAttributeBase(UBaseAttributeSet::GetHealthAttribute(), NewHealth);
	}

	CurrentAnimState = EMageAnimState::Casting;
	if (HealCastAnimations.Num() > 0)
	{
		UAnimSequence* Anim = HealCastAnimations[FMath::RandRange(0, HealCastAnimations.Num() - 1)];
		if (Anim)
		{
			GetMesh()->PlayAnimation(Anim, false);
		}
	}
	if (HealSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HealSound, GetActorLocation(), 0.25f);
	}
	OnHealCast(BestTarget);

	bHealOnCooldown = true;
	float Jitter = FMath::FRandRange(0.f, HealCooldown * 0.5f);
	GetWorldTimerManager().SetTimer(
		HealCooldownTimer,
		[this]()
		{
			bHealOnCooldown = false;
			if (CurrentAnimState == EMageAnimState::Casting)
			{
				CurrentAnimState = EMageAnimState::Idle;
			}
		},
		HealCooldown + Jitter,
		false
	);
}
