#pragma once

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	ECS_Unequipped UMETA(DisplayName = "Unequipped"),
	ECS_EquippedMagicFire UMETA(DisplayName = "EquippedMagicFire"),
	ECS_EquippedMagicMelee UMETA(DisplayName = "EquippedMagicMelee"),
	ECS_Charging UMETA(DisplayName = "Charging")
};

UENUM(BlueprintType)
enum class EActionState : uint8
{
	EAS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	EAS_HitReaction UMETA(DisplayName = "HitReaction"),
	EAS_Aiming UMETA(DisplayName = "Aiming"),
	EAS_Melee UMETA(DisplayName = "Melee"),
	EAS_EquippingWeapon UMETA(DisplayName = "EquippingWeapon"),
	EAS_Dodge UMETA(DisplayName = "Dodge"),
	EAS_Dead UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ERunState : uint8
{
	ERS_Normal UMETA(DisplayName = "RunNormal"),
	ERS_Fast UMETA(DisplayName = "RunFast"),
	ERS_Aggro UMETA(DisplayName = "RunAggro")
};

UENUM(BlueprintType)
enum EDeathPose
{
	EDP_Death1 UMETA(DisplayName = "Death1"),
	EDP_Death2 UMETA(DisplayName = "Death2"),
	EDP_Death3 UMETA(DisplayName = "Death3"),
	EDP_Death4 UMETA(DisplayName = "Death4"),
	EDP_Death5 UMETA(DisplayName = "Death5"),
	EDP_Death6 UMETA(DisplayName = "Death6"),
	EDP_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	EES_NoState UMETA(DisplayName = "NoState"),
	EES_Dead UMETA(DisplayName = "Dead"),
	EES_Patrolling UMETA(DisplayName = "Patrolling"),
	EES_Chasing UMETA(DisplayName = "Chasing"),
	EES_Attacking UMETA(DisplayName = "Attacking"),
	EES_Engaged UMETA(DisplayName = "Engaged")
};