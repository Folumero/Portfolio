// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "InputActionValue.h"
#include "CharacterTypes.h"
#include "Josep/Interfaces/PickupInterface.h"
#include "NiagaraComponent.h"
#include "Components/TimelineComponent.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "PlayerCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class USphereComponent;
class AItem;
class ASoul;
//class UGroomComponent;
class UAnimMontage;
class UPlayerOverlay;

USTRUCT(BlueprintType)
struct FAbilityUnlockInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AbilityName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SoulCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsUnlocked;

	FAbilityUnlockInfo() : AbilityName(TEXT("")), SoulCost(0), bIsUnlocked(false) {}

	FAbilityUnlockInfo(const FString& InName, const int32 InCost, const bool InIsUnlocked = false)
		: AbilityName(InName), SoulCost(InCost), bIsUnlocked(InIsUnlocked) {}
};

UENUM(BlueprintType)
enum class EDodgeDirection : uint8
{
	None,
	Left,
	Right,
	Forward,
	Backward
};

UENUM(BlueprintType)
enum class ETargetSwitchDirection
{
	Left,
	Right
};

UENUM(BlueprintType)
enum class ECurrentAttackType
{
	None UMETA(DisplayName = "None"),
	Melee UMETA(DisplayName = "Melee"),
	Fire UMETA(DisplayName = "Fire"),
	Lightning UMETA(DisplayName = "Lightning"),
	Gravity UMETA(DisplayName = "Gravity"),
	Vital UMETA(DisplayName = "Vital")
};

UENUM(BlueprintType)
enum class ERMBAction
{
	LightningAttack,
	GravityAttack,
	VitalAttack,
	// Add more actions as needed
};


UCLASS()
class PROYECTO4_API APlayerCharacter : public ABaseCharacter, public IPickupInterface
{
	GENERATED_BODY()

public:
	APlayerCharacter();



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundCue* ShieldBreakSoundCue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundCue* VitalChargeSoundCue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundCue* ElectricChargeSoundCue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
	UAudioComponent* CharacterAudioComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
	UAudioComponent* ChargeAudioComponent;

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/*<Widget AimTarget>*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WidgetAim")
	UWidgetComponent* TargetSelectionWidget;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> TargetSelectionWidgetClass;
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* myAnimationName = nullptr;
	float WidgetHeightAboveActor;
	/*</Widget AimTarget>*/

	bool bIsWidgetAnimationPlaying;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void AddImpulseMelee();

	UFUNCTION(BlueprintCallable, Category = "Materials")
	void ChangeAttackMaterials();
	UFUNCTION(BlueprintCallable, Category = "Materials")
	void RevertMaterials();

	void ResetChangeCooldown();

	bool CooldownChange = false;

	void ChangeGamepadLeft();
	UFUNCTION(BlueprintImplementableEvent, Category = "Dead")
	void DeadHudAnim();
	UFUNCTION(BlueprintImplementableEvent, Category = "Dead")
	void AddVFXLightningCharge();

	UFUNCTION(BlueprintCallable, Category = "Outline")
	void AddOutline(AActor* OtherActor);
	UFUNCTION(BlueprintCallable, Category = "Outline")
	void RemoveOutline(AActor* OtherActor);
	UFUNCTION(BlueprintImplementableEvent, Category = "Vital")
	void AddVital();

	UFUNCTION(BlueprintImplementableEvent, Category = "Vital")
	void AddChargedVital();

	UFUNCTION(BlueprintImplementableEvent, Category = "Vital")
	void RemoveChargedVital();

	UFUNCTION(BlueprintImplementableEvent, Category = "Vital")
	void RemoveVital();

	UFUNCTION(BlueprintImplementableEvent, Category = "Vital")
	void ActivateShieldVFX();

	UFUNCTION(BlueprintImplementableEvent, Category = "Vital")
	void DeactivateShieldVFX();

	void ToggleAimState();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	/* <IHitInterface> */
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	/* </IHitInterface> */

	/* <IPickupInterface> */
	virtual void SetOverlappingItem(AItem* Item) override;
	UFUNCTION(BlueprintCallable)
	void AddSoulsInterface(int32 Souls);
	virtual void AddSouls(ASoul* Soul) override;
	virtual void AddGold(int32 GoldAdded) override;
	UFUNCTION(BlueprintImplementableEvent, Category = "Abilities")
	void HandleGravitationalThrowEnd(const FVector& EndTransform, AActor* CollidingActor);
	UFUNCTION(BlueprintImplementableEvent, Category = "Abilities")
	void ChangeToLightning();
	UFUNCTION(BlueprintImplementableEvent, Category = "Abilities")
	void ChangeToGravity();
	UFUNCTION(BlueprintImplementableEvent, Category = "Abilities")
	void ChangeToVital();
	UFUNCTION(BlueprintImplementableEvent, Category = "Abilities")
	void ChangeToLightning2();
	UFUNCTION(BlueprintImplementableEvent, Category = "Abilities")
	void ChangeToGravity2();
	UFUNCTION(BlueprintImplementableEvent, Category = "Abilities")
	void NoManaAnim();
	UFUNCTION(BlueprintImplementableEvent, Category = "Boss")
	void PushBossInteraction(AActor* InstigatorE);

	TSet<AActor*> HitActors;

	FTimerHandle ThrowTimerHandle;
	FTimerHandle VitalEffectTimerHandle;
	FTimerHandle CooldownShield;
	FTimerHandle CooldownTimerHandle;
	FTimerHandle ChangeAbility;
	FVector StartPosition;
	FVector TargetPosition;
	FVector LastValidLocation;
	bool bCanActivateShield = true;
	float ThrowDuration;
	float ElapsedTime;
	UFUNCTION(BlueprintImplementableEvent, Category = "Tep")
	void SpawnLightningTep(FVector SpawnLocation);
	void TriggerScreenShake();
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void CameraShake();
	/* </Fijado de Cámara> */
	UPROPERTY(EditAnywhere, Category = "FijadoCamara")
	float MinimumDistance = 100.0f;
	UPROPERTY(EditAnywhere, Category = "FijadoCamara")
	float MaximusDistance = 600.0f;
	UPROPERTY(EditAnywhere, Category = "FijadoCamara")
	float PitchWhenFar = -10.0f; 
	UPROPERTY(EditAnywhere, Category = "FijadoCamara")
	float PitchWhenNear = 20.0f;
	UPROPERTY(EditAnywhere, Category = "FijadoCamara")
	float AimHeightIncrease = 50.0f;
	UPROPERTY(EditAnywhere, Category = "FijadoCamara")
	float TargetPitch = -10;
	UPROPERTY(EditAnywhere, Category = "FijadoCamara")
	float OriginalCameraBoomHeight = 75.0f;
	UPROPERTY(EditAnywhere, Category = "FijadoCamara")
	float HeightAboveEnemy = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Gravedad")
	float DistanceBehind = 1000.0f;

	/* </IPickupInterface> */
	

	UPROPERTY(EditAnywhere)
	float CameraRotationSpeed = 20.f;
	UPROPERTY(EditAnywhere)
	float CameraDistanceFromTarget = 30.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check")
	bool bChangeBlock = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check")
	bool isColliderActivated = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check")
	bool isAiming = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check")
	bool bIsCharging = false;

	bool bNoMana = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check")
	bool bCanAttack = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check")
	bool bCargaVida = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check")
	bool bCargaRayo = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check")
	bool bCantGethit = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check")
	bool bCanDodge = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check")
	bool bCanDash = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check")
	bool bIsDashing = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check")
	bool bcanRotate = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check")
	bool bShouldOrientTowardsTarget = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check")
	bool bShouldMoveForward = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UNiagaraComponent* ActiveParticleComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	UNiagaraSystem* GravitySystemTemplate;

	UPROPERTY(EditAnywhere, Category = "Effects")
	UNiagaraSystem* GrabParticleEffectTemplate;

	float ChargeStartTime = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float DashDistance = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InputBufferTime = 0.5f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dash")
	UTimelineComponent* DashTimeline;

	UFUNCTION()
	void DashTick(float Value);

	UFUNCTION(BlueprintCallable, Category = "Attack")
	void SetAttackToNone();

	UFUNCTION(BlueprintNativeEvent, Category = "Attack")
	void FinAim();
	virtual void FinAim_Implementation();

	UFUNCTION(BlueprintCallable, Category = "Attack")
	void MoveForward(float ScaleValue);

	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	UMaterialInterface* AttackMaterial1;

	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	UMaterialInterface* AttackMaterial2;

	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	UMaterialInterface* AttackMaterial3;

	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	UMaterialInterface* InvisibleMaterial;


	/*-------------------------<Cost Abilities>---------------------------------------*/
	UPROPERTY(EditAnywhere, Category = "AbilitiesCost")
	float DodgeCost = 10.f;
	UPROPERTY(EditAnywhere, Category = "AbilitiesCost")
	float LightningCost = 10.f;
	UPROPERTY(EditAnywhere, Category = "AbilitiesCost")
	float ChargeLightningCost = 25.f;
	UPROPERTY(EditAnywhere, Category = "AbilitiesCost")
	float GravityCost = 10.f;
	UPROPERTY(EditAnywhere, Category = "AbilitiesCost")
	float ChargeGravityCost = 10.f;
	UPROPERTY(EditAnywhere, Category = "AbilitiesCost")
	float VitalCost = 5.f;
	UPROPERTY(EditAnywhere, Category = "AbilitiesCost")
	float ChargeVitalCost = 5.f;
	UPROPERTY(EditAnywhere, Category = "AbilitiesDamage")
	float MeleeDamage = 20.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitiesDamage")
	float MeleeChargedDamage = 35.f;
	UPROPERTY(EditAnywhere, Category = "AbilitiesDamage")
	float MeleeChargedDamageInicial = 35.f;
	UPROPERTY(EditAnywhere, Category = "AbilitiesDamage")
	float GravityDamage = 50.f;
	UPROPERTY(EditAnywhere, Category = "AbilitiesDamage")
	float GravityPoweredDamage = 75.f;

	/*-------------------------<Gravity>---------------------------------------*/

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravedad")
	TArray<AActor*> GravitationalObjects;
	UPROPERTY(VisibleAnywhere, Category = "Gravedad")
	AActor* GravitationalObject;

	UPROPERTY(EditAnywhere, Category = "Gravedad")
	TSubclassOf<AActor> Tep;

	UPROPERTY(EditAnywhere, Category = "Gravedad")
	float LevitationHeight = 100.f;
	UPROPERTY(EditAnywhere, Category = "Gravedad")
	float LevitationOffsetX = 100.f;
	UPROPERTY(EditAnywhere, Category = "Gravedad")
	float LevitationOffsetY = 100.f;

	UPROPERTY(EditAnywhere, Category = "Gravedad")
	float ThrowForce = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check")
	bool bIsGravitationalPullActive = false;

	void ThrowGravitationalObjects(AActor* Target);

	void UpdateThrowPosition();

	UFUNCTION(BlueprintCallable)
	void DeactivateGravityAbility();

	UFUNCTION(BlueprintCallable)
	void OnWeaponColliderActivated();

	UFUNCTION(BlueprintCallable)
	void OnWeaponColliderDeactivated();

	UFUNCTION(BlueprintCallable)
	void OnLightningColliderActivated();

	UFUNCTION(BlueprintCallable)
	void OnLightningColliderDeactivated();

	UFUNCTION(BlueprintCallable)
	void AdjustWeaponBox(const FVector& NewSize);

	UFUNCTION(BlueprintCallable)
	void AdjustLightningBox(const FVector& NewSize);

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void SpawnLightningStrikeFromAnimation();

	void LanzamientoRayo(FVector& StartPoint, FVector& EndPoint, AActor* Target);

	AActor* FindClosestEnemyAround(AActor* Target);

	void HandleHealing();

	void HandleManaVital();

	UFUNCTION(BlueprintCallable)
	void DashImpulse();

	UFUNCTION(BlueprintCallable)
	void FindAndSetClosestEnemyInSight();

	UFUNCTION(BlueprintCallable)
	void StopCharging();
protected:
	UPROPERTY(EditAnywhere, Category = "Combat")
	UCurveFloat* ChargeAttackCurve;

	UPROPERTY()
	UTimelineComponent* ChargeAttackTimeline;

	FVector ChargeAttackInitialPosition;
	FVector ChargeAttackTargetPosition;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitiesDamage")
	float ImpulseForce = 500.0f;

	UFUNCTION()
	void HandleTimelineProgress(float Value);

	void ConfigureChargeAttackTimeline();

	virtual void BeginPlay() override;

	void SwitchTargetAxis(const FInputActionValue& AxisValue);

	void EnableTrigger();

	void ChangeTargetByDirection(ETargetSwitchDirection SwitchDirection);

	void LockOn(bool bSwitchLeft);

	void LockOn();

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnWeaponColliderOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input);
	UInputMappingContext* SlashContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input);
	UInputAction* MovementAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input);
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input);
	UInputAction* EKeyAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input);
	UInputAction* ChangeAttackLightningAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input);
	UInputAction* ChangeSecondaryGamepadRightAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input);
	UInputAction* ChangeSecondaryGamepadLeftAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input);
	UInputAction* ChangeAttackGravityAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input);
	UInputAction* ChangeAttackVitalAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input);
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input);
	UInputAction* SecondaryAttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input);
	UInputAction* DodgeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input);
	UInputAction* LockAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input);
	UInputAction* AimAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input);
	UInputAction* ChangeTargetAction;

	/*
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input);
	UInputAction* DodgeComboAction;
	*/
	/*
	Callbacks for inputs
	*/
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartCharging();
	virtual void Attack() override;
	void OrientTowards(AActor* Target);
	virtual void AttackFire();
	virtual void AttackGravity();
	virtual void AttackLightning();
	void Dodge();
	void Lock();
	//void DodgeCombo(const FInputActionValue& Value);

	void ChangeGamepadRight();

	/* Combat */
	void ChangeLightning();
	UFUNCTION(BlueprintCallable)
	void ChangeGravity();
	UFUNCTION(BlueprintCallable)
	void ChangeVital();
	void Dash();
	UFUNCTION(BlueprintCallable)
	void CancelDash();
	void OnAttackReleased();
	void PerformRegularAttack();
	void ResetAttackCooldown();
	void ReleasedChargedAttack();
	void ExecuteChargedAttack();
	void AttackMeleeCombo();
	void AttackMagicCombo();
	void AttackCombo(ECurrentAttackType Type);
	virtual void AttackEnd() override;
	virtual void DodgeEnd() override;
	virtual bool CanAttack() override;
	UFUNCTION(BlueprintCallable)
	void AnimAttackEnd();
	bool CanDisarm();
	bool CanArm();
	void Disarm();
	void Arm();
	void PlayEquipMontage(const FName SectionName);
	virtual void Die() override;
	UFUNCTION(BlueprintImplementableEvent, Category = "Nivel")
	void CambioNivel();
	bool HasEnoughMana();
	bool IsOccupied();
	void PlayDodgeAnimation(EDodgeDirection DodgeDirection);
	EDodgeDirection CalculateDodgeDirection(const FVector& Direction);

	UFUNCTION(BlueprintCallable)
	void AttachWeaponToBack();

	UFUNCTION(BlueprintCallable)
	void AttachWeaponToHand();

	UFUNCTION(BlueprintCallable)
	void FinishEquipping();

	UFUNCTION(BlueprintCallable)
	void HitReactEnd();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EDodgeDirection DodgeDirection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECurrentAttackType AttackType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bGravityUnlocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bVitalUnlocked = false;

	bool bIsTargeting = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<AActor*> EnemiesInRange;

	UPROPERTY(EditAnywhere, Category = "Dash")
	UCurveFloat* DashCurve;

	FVector DashStartLocation;

	FVector DashTargetLocation;

	void InitializeDashTimeline();

	ERMBAction CurrentRMBAction = ERMBAction::LightningAttack;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UBoxComponent* WeaponCollider;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UBoxComponent* LightningChargedCollider;

	UFUNCTION(BlueprintCallable)
	void SetHUDMana();

	UFUNCTION(BlueprintCallable)
	void SetHUDHealth();

private:
	bool bShouldTriggerScreenShake = false;
	bool IsUnoccupied();
	void InitializePlayerOverlay();
	void SelectNearestGravitationalObject();
	void ThrowGravitationalObjectsWithoutEnemy();
	FVector2D Direccion;
	FVector DireccionMov;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	AActor* NearestGravitationalObject = nullptr;

	/* Character components */

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* ViewCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	USphereComponent* DetectionSphere;

	UPROPERTY(VisibleInstanceOnly)
	AItem* OverlappingItem;

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* EquipMontage;

	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	ERunState RunState = ERunState::ERS_Normal;

	bool bVitalActivated = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	EActionState ActionState = EActionState::EAS_Unoccupied;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UPlayerOverlay* PlayerOverlay;

	UPROPERTY(EditInstanceOnly, Category = "Target")
	TArray<AActor*> EnemyTargets;



	UPROPERTY(EditAnywhere)
	double TargetRadius = 200.f;

	bool bCanTrigger = true;
	const float TriggerCooldown = 0.5f; // Set your desired cooldown time here
	const float ChargeAttackThreshold = 0.5f; // Seconds

	FTimerHandle TimerHandle_TriggerCooldown;
	FTimerHandle TimerHandle_ChargeAttack;
	FTimerHandle TimerHandle_Healing;
	FTimerHandle TimerHandle_UsingVital;

	
	//float ChargeDuration = 0.0f;
	const float MaxChargeDuration = 2.0f;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AWeapon> WeaponClass;

	int32 LightningStrikeCount = 0;

	/*-------------------------<Input Buffering>---------------------------------------*/
	bool bDashInputBuffered = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Check")
	bool bAttackInputBuffered = false;
	bool bIsGravityCharged = false;
	float LastInputTimeDash = 0.0f;
	float LastInputTimeAttack = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Check")
	bool bIsAttackOnCooldown = false;
	bool bShieldActive = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Check")
	bool bIsAlreadyOnAttack = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Check")
	bool bHasNotReleasedAttack = false;
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TArray<FAbilityUnlockInfo> AbilitiesToUnlock;
	UFUNCTION(BlueprintCallable)
	void UnlockAbility(FString AbilityName);
	UFUNCTION(BlueprintCallable)
	bool CanBuyAbility(FString AbilityName);
	bool IsAbilityUnlocked(FString AbilityName) const;
	void ActivateShield();
	void CanActivateShield();
	void ActivateVitalAttackEffects();
	void DeactivateVitalAttackEffects();
	UFUNCTION(BlueprintCallable)
	void AddLightning();
	UFUNCTION(BlueprintCallable)
	void SpawnLightning();
	UFUNCTION(BlueprintCallable)
	void ResetLightningAttack();
	void SpawnOrAdjustDamageCollider(int32 NumLightnings);
	float CalculateColliderSizeBasedOnLightning(int32 NumLightnings);
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
	FORCEINLINE EActionState GetActionState() const { return ActionState; }
	FORCEINLINE ERunState GetRunState() const { return RunState; }
};
