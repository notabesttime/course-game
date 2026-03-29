#include "CombatAttributeUtils.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "BaseAttributeSet.h"

namespace CombatAttributeUtils
{
	bool TryGetAbilityData(AActor* TargetActor, UAbilitySystemComponent*& OutASC, const UBaseAttributeSet*& OutAttrSet)
	{
		OutASC = nullptr;
		OutAttrSet = nullptr;

		if (!TargetActor)
		{
			return false;
		}

		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(TargetActor))
		{
			OutASC = ASI->GetAbilitySystemComponent();
			if (OutASC)
			{
				OutAttrSet = OutASC->GetSet<UBaseAttributeSet>();
			}
		}

		return OutASC && OutAttrSet;
	}

	bool ApplyHealthDelta(AActor* TargetActor, float Delta)
	{
		UAbilitySystemComponent* ASC = nullptr;
		const UBaseAttributeSet* AttrSet = nullptr;
		if (!TryGetAbilityData(TargetActor, ASC, AttrSet))
		{
			return false;
		}

		const float NewHealth = FMath::Clamp(AttrSet->GetHealth() + Delta, 0.f, AttrSet->GetMaxHealth());
		ASC->SetNumericAttributeBase(UBaseAttributeSet::GetHealthAttribute(), NewHealth);
		return true;
	}

	bool ApplyManaDelta(AActor* TargetActor, float Delta)
	{
		UAbilitySystemComponent* ASC = nullptr;
		const UBaseAttributeSet* AttrSet = nullptr;
		if (!TryGetAbilityData(TargetActor, ASC, AttrSet))
		{
			return false;
		}

		const float NewMana = FMath::Clamp(AttrSet->GetMana() + Delta, 0.f, AttrSet->GetMaxMana());
		ASC->SetNumericAttributeBase(UBaseAttributeSet::GetManaAttribute(), NewMana);
		return true;
	}
}
