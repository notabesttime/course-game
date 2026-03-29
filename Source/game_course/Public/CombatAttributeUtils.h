#pragma once

#include "CoreMinimal.h"

class AActor;
class UAbilitySystemComponent;
class UBaseAttributeSet;

namespace CombatAttributeUtils
{
	GAME_COURSE_API bool TryGetAbilityData(AActor* TargetActor, UAbilitySystemComponent*& OutASC, const UBaseAttributeSet*& OutAttrSet);
	GAME_COURSE_API bool ApplyHealthDelta(AActor* TargetActor, float Delta);
	GAME_COURSE_API bool ApplyManaDelta(AActor* TargetActor, float Delta);
}
