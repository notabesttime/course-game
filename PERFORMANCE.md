# Performance Optimization Suggestions

## Critical Bottlenecks

### 1. EnemySpawner: GetAllActorsOfClass() on every spawn
**Files:** EnemySpawner.cpp:49-68 (CountWarriors/CountMages/CountBrutes)

`GetAllActorsOfClass()` iterates every actor in the world. Called 3 times per spawn attempt, per spawner — scales poorly as enemy count grows.

**Fix:** Maintain static live-counters (increment on spawn, decrement on death) instead of scanning the world.
**Consequence:** Need to hook into enemy death events; counter must handle edge cases (level unload, etc.).

---

### 2. PlayerCharacter: Cursor trace every single frame
**File:** PlayerCharacter.cpp:85-112 (UpdateHoveredEnemy in Tick)

`GetHitResultUnderCursorForObjects()` runs a physics trace 60 times per second. Also creates a `TArray<TEnumAsByte<EObjectTypeQuery>>` on the stack every frame.

**Fix:** Throttle to 10-20 Hz. Cache the ObjectTypes array as a member.
**Consequence:** Minimal — hover highlight updates at 20 Hz is imperceptible to the player.

---

### 3. MinionMage: Overlap sphere query every frame
**File:** MinionMage.cpp:180+ (TryHealAlly called from Tick)

Every mage runs `OverlapMultiByObjectType()` with a sphere query every frame. With 10 mages this is 10 expensive physics queries per frame — most of which early-exit due to cooldown anyway.

**Fix:** Move the overlap query to a timer (0.25s interval) or check cooldown before querying.
**Consequence:** Healing target detection delayed by up to 250ms — not noticeable for a heal ability.

---

### 4. BaseCompanion: FindNearestEnemy() every frame
**File:** BaseCompanion.cpp:103-138 (called from UpdateBehavior in Tick)

Same pattern as mage healing — overlap sphere query every tick per companion.

**Fix:** Cache current target for 200-500ms before re-searching. Only re-search when target dies or leaves range.
**Consequence:** Target switching slightly delayed. Not noticeable in gameplay.

---

### 5. GameHUD: UpdateSpawnerIndicators() every frame
**File:** GameHUD.cpp:140-244

Uses `TActorIterator` to find all spawners every frame, calls `ProjectWorldLocationToScreen()` and `GetPlayerViewPoint()` multiple times per spawner, builds rotation matrices per indicator.

**Fix:** Cache spawner list (spawners don't change mid-frame). Call `GetPlayerViewPoint()` once and reuse. Throttle to 30 Hz if needed.
**Consequence:** Indicator positions update slightly less often — invisible at 30 Hz.

---

## High-Priority Bottlenecks

### 6. MinionWarrior/Brute: TryAttackPlayer() every frame
**Files:** MinionWarrior.cpp:106-166, MinionBrute.cpp (similar pattern)

Every warrior/brute does `Cast<APlayerCharacter>(GetPlayerCharacter())`, computes `FVector::Dist()` (sqrt), and runs a cast chain for damage — every frame.

**Fix:** Cache player reference in BeginPlay. Use `DistSquared()` instead of `Dist()`. Only rotate + apply damage when in range.
**Consequence:** None — same behavior, less CPU.

---

### 7. BaseEnemy: Runtime material creation per enemy
**File:** BaseEnemy.cpp:28-44 (CreateHighlightMaterial)

Each enemy creates a new `UMaterial` with `PostEditChange()` (triggers shader compilation) in BeginPlay. Editor-only but still expensive per spawn.

**Fix:** Create the highlight material once (static cache) or use a pre-made material asset.
**Consequence:** All enemies share the same highlight material — which is the intent anyway.

---

### 8. CameraOcclusionComponent: Line trace every frame
**File:** CameraOcclusionComponent.cpp:13-48

`LineTraceMultiByChannel()` runs every tick. Also does redundant double-casts for the controller.

**Fix:** Throttle to 10-15 Hz. Cache the player controller reference. Skip trace when opacity has reached its target.
**Consequence:** Occlusion fade reacts slightly slower — not visible at 15 Hz.

---

### 9. HealingOrb: sqrt in Tick
**File:** HealingOrb.cpp:33-75

Uses `FVector::Dist()` (sqrt) and `GetSafeNormal()` (another sqrt) every frame per orb.

**Fix:** Use `SizeSquared()` for distance check, compute sqrt only once when needed for movement direction.
**Consequence:** None — same result, one fewer sqrt per frame per orb.

---

## Medium-Priority

### 10. GameTimerWidget: FString::Printf every frame
**File:** GameTimerWidget.cpp:65-89

Formats and sets timer text every frame, triggering widget invalidation.

**Fix:** Only update text when the displayed value actually changes (round to 0.1s, compare).
**Consequence:** None visible — text updates at 10 Hz instead of 60 Hz.

---

### 11. GlassSphereHealthWidget: Vertex generation every frame
**File:** GlassSphereHealthWidget.cpp:270-323

Rebuilds vertex/index arrays with trig (sin/cos) operations in NativePaint every frame.

**Fix:** Cache the generated mesh; only regenerate when health/mana value changes.
**Consequence:** Requires tracking dirty state. Higher code complexity.

---

### 12. No object pooling for frequently spawned actors
**Files:** HealingOrb, mage projectiles, minion enemies

Actors are spawned and destroyed constantly, causing allocation churn and GC pressure.

**Fix:** Implement an actor pool — deactivate instead of destroy, reactivate instead of spawn.
**Consequence:** Significant refactor. Pool sizing needs tuning. Higher memory baseline but smoother frame times.

---

## Low-Priority / Code Quality

### 13. Repeated cast chains for damage application
**Files:** MinionWarrior, MinionBrute, MinionMage, BaseCompanion

Pattern: `Cast<IAbilitySystemInterface>` -> `GetAbilitySystemComponent()` -> `GetSet<UBaseAttributeSet>()` repeated in every attack function.

**Fix:** Cache the ASC/AttributeSet pointers on the target, or provide a helper `ApplyDamage(AActor*, float)`.
**Consequence:** Cleaner code, fewer casts per attack.

---

### 14. Redundant IsValid() after Cast
**Files:** Various

`Cast<>()` already returns nullptr for pending-kill actors in most UE configs. Adding `IsValid()` after is redundant.

**Fix:** Remove the extra check or keep only `IsValid()` if you specifically need pending-kill protection.
**Consequence:** Negligible perf gain, cleaner code.

---

## Recommended Priority

| Phase | Items | Effort | Expected Impact |
|-------|-------|--------|-----------------|
| Quick wins | #1, #2, #3, #6, #9 | 1-2 hrs | Large — removes heaviest per-frame costs |
| Medium effort | #4, #5, #7, #8, #10 | 2-4 hrs | Noticeable — reduces frame spikes during waves |
| System-level | #11, #12, #13 | 4-8 hrs | Smooths high-enemy-count scenarios |
