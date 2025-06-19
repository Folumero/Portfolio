// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/PlayerCharacter.h"

#include "Components/InputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/AttributeComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
//#include "GroomVisualizationData.h"
#include "Items/Item.h"
#include "/Items/Weapons/Weapon.h"
#include "Animation/AnimMontage.h"
#include "/HUD/PlayerHUD.h"
#include "/HUD/PlayerOverlay.h"
#include "/Items/Soul.h"
#include "GeometryCollection/GeometryCollectionComponent.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 300.f;

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(CameraBoom);

	DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
	DetectionSphere->SetupAttachment(GetRootComponent()); 
	DetectionSphere->InitSphereRadius(300.0f);
	DetectionSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);

	DashTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DashTimeline"));

	ChargeAttackTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("ChargeAttackTimeline"));

	WeaponCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("WeaponCollider"));
	WeaponCollider->SetupAttachment(GetMesh());
	WeaponCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponCollider->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	WeaponCollider->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponCollider->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2, ECollisionResponse::ECR_Ignore);

	LightningChargedCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("LightningChargedCollider"));
	LightningChargedCollider->SetupAttachment(GetMesh());
	LightningChargedCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LightningChargedCollider->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	LightningChargedCollider->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	LightningChargedCollider->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2, ECollisionResponse::ECR_Ignore);

	TargetSelectionWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("TargetSelectionWidget"));
	TargetSelectionWidget->SetupAttachment(RootComponent);
	TargetSelectionWidget->SetWidgetSpace(EWidgetSpace::World);
	TargetSelectionWidget->SetVisibility(false);
	TargetSelectionWidget->SetDrawSize(FVector2D(100, 100));
	TargetSelectionWidget->SetWidgetClass(TargetSelectionWidgetClass);

	CharacterAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("CharacterAudioComponent"));
	CharacterAudioComponent->SetupAttachment(RootComponent);
	CharacterAudioComponent->bAutoActivate = false;

	ChargeAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ChargedAudioComponent"));
	ChargeAudioComponent->SetupAttachment(RootComponent);
	ChargeAudioComponent->bAutoActivate = false;
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ActionState == EActionState::EAS_Unoccupied && !bCanAttack)
	{
		bCanAttack = true;
	}

	if (bIsGravitationalPullActive && NearestGravitationalObject)
	{
		if (ViewCamera)
		{
			FVector CameraBackwardVector = -ViewCamera->GetForwardVector();
			FVector LevitationTarget = GetActorLocation() + CameraBackwardVector * DistanceBehind + FVector(0, 0, LevitationHeight);
			NearestGravitationalObject->SetActorLocation(FMath::Lerp(NearestGravitationalObject->GetActorLocation(), LevitationTarget, 0.05f));
		}
	} 

	if (Attributes && PlayerOverlay && !bVitalActivated && !bCargaVida && !bCargaRayo)
	{
		Attributes->RegenMana(DeltaTime);
		PlayerOverlay->SetManaBarPercent(Attributes->GetManaPercent());
	}

	if (CombatTarget && !AimTarget && bShouldOrientTowardsTarget)
	{
		OrientTowards(CombatTarget);
	}

	if (CombatTarget && !AimTarget && bShouldMoveForward)
	{
		float DistanceToTarget = FVector::Distance(GetActorLocation(), CombatTarget->GetActorLocation());

		if (DistanceToTarget > 200.0f)
		{
			FVector Direction = (CombatTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();

			AddMovementInput(Direction, 1.0f);
		}
	}

	if (bShouldMoveForward && !AimTarget && !CombatTarget) {
		AddMovementInput(GetActorForwardVector(), 1.0f);
	}

	if (bDashInputBuffered && (GetWorld()->GetTimeSeconds() - LastInputTimeDash) <= InputBufferTime)
	{
		if (bCanDash)
		{
			Dash();
			bDashInputBuffered = false;
			bAttackInputBuffered = false;
		}
	}

	if (bAttackInputBuffered && (GetWorld()->GetTimeSeconds() - LastInputTimeAttack) <= InputBufferTime && bCanAttack && !bIsCharging)
	{
		if (!bCanDash)
		{
			CancelDash();
		}
		if (CanAttack() || bCanAttack)
		{
			bCanAttack = false;
			PerformRegularAttack();
			LastInputTimeAttack = GetWorld()->GetTimeSeconds(); 
			bAttackInputBuffered = false; 
		}
	}

	
	if (AimTarget)
	{
		TargetSelectionWidget->SetVisibility(true);
		if (!bIsWidgetAnimationPlaying)
		{
			if (TargetSelectionWidget->IsVisible() && ViewCamera)
			{
				if (ACharacter* Character = Cast<ACharacter>(AimTarget))
				{
					UCapsuleComponent* EnemyCapsuleComponent = Character->GetCapsuleComponent();
					float CapsuleHalfHeight = EnemyCapsuleComponent->GetUnscaledCapsuleHalfHeight();
					FVector ComponentScale = EnemyCapsuleComponent->GetComponentScale();
					
					float ScaledFullHeight = CapsuleHalfHeight * ComponentScale.Z;
					WidgetHeightAboveActor = ScaledFullHeight * HeightAboveEnemy;
				}
				
				if (ViewCamera)
				{
					FRotator CameraRotation = ViewCamera->GetComponentRotation();
					FRotator WidgetRotation = FRotator(0.0f, CameraRotation.Yaw, 0.0f);
					TargetSelectionWidget->SetWorldRotation(WidgetRotation);
				}
			}

			bIsWidgetAnimationPlaying = true;
		}
		TargetSelectionWidget->SetWorldLocation(AimTarget->GetActorLocation() + FVector(0, 0, WidgetHeightAboveActor));
		FVector CurrentBoomPosition = CameraBoom->GetRelativeLocation();
		FVector TargetBoomPosition = FVector(CurrentBoomPosition.X, CurrentBoomPosition.Y, OriginalCameraBoomHeight + AimHeightIncrease);

		FVector NewBoomPosition = FMath::VInterpTo(CurrentBoomPosition, TargetBoomPosition, DeltaTime, 5.0f);
		CameraBoom->SetRelativeLocation(NewBoomPosition);

		FVector TargetLocation = AimTarget->GetActorLocation();
		FVector CameraLocation = CameraBoom->GetComponentLocation();
		FRotator TargetRotation = (TargetLocation - CameraLocation).Rotation();

		float DistanceToTarget = FVector::Distance(CameraLocation, TargetLocation);

		float T = (DistanceToTarget - MinimumDistance) / (MaximusDistance - MinimumDistance);
		T = FMath::Clamp(T, 0.0f, 1.0f);
		float ClampedPitch = FMath::Lerp(PitchWhenNear, PitchWhenFar, T);

		FRotator NewControlRotation = FRotator(ClampedPitch, TargetRotation.Yaw, 0);
		FRotator CurrentControlRotation = GetController()->GetControlRotation();
		FRotator SmoothedRotation = FMath::RInterpTo(CurrentControlRotation, NewControlRotation, DeltaTime, 10.0f);
		GetController()->SetControlRotation(SmoothedRotation);

		if (!bCanAttack) {
			OrientTowards(AimTarget);
		}

		if (bShouldMoveForward) {
			if (DistanceToTarget > 200.0f)
			{
				FVector Direction = (AimTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
				AddMovementInput(Direction, 1.0f);
			}
		}
	}
	else
	{
		FVector CurrentBoomPosition = CameraBoom->GetRelativeLocation();
		FVector TargetBoomPosition = FVector(CurrentBoomPosition.X, CurrentBoomPosition.Y, OriginalCameraBoomHeight);
		FVector NewBoomPosition = FMath::VInterpTo(CurrentBoomPosition, TargetBoomPosition, DeltaTime, 5.0f);
		CameraBoom->SetRelativeLocation(NewBoomPosition);
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &APlayerCharacter::AttackMeleeCombo);
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Started, this, &APlayerCharacter::Dash);
		EnhancedInputComponent->BindAction(LockAction, ETriggerEvent::Completed, this, &APlayerCharacter::LockOn);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &APlayerCharacter::ToggleAimState);
		EnhancedInputComponent->BindAction(ChangeTargetAction, ETriggerEvent::Started, this, &APlayerCharacter::SwitchTargetAxis);
		EnhancedInputComponent->BindAction(ChangeAttackLightningAction, ETriggerEvent::Started, this, &APlayerCharacter::ChangeLightning);
		EnhancedInputComponent->BindAction(ChangeAttackGravityAction, ETriggerEvent::Started, this, &APlayerCharacter::ChangeGravity);
		EnhancedInputComponent->BindAction(ChangeAttackVitalAction, ETriggerEvent::Started, this, &APlayerCharacter::ChangeVital);
		EnhancedInputComponent->BindAction(SecondaryAttackAction, ETriggerEvent::Started, this, &APlayerCharacter::AttackMagicCombo);

		EnhancedInputComponent->BindAction(ChangeSecondaryGamepadRightAction, ETriggerEvent::Started, this, &APlayerCharacter::ChangeGamepadRight);
		EnhancedInputComponent->BindAction(ChangeSecondaryGamepadLeftAction, ETriggerEvent::Started, this, &APlayerCharacter::ChangeGamepadLeft);

		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Completed, this, &APlayerCharacter::OnAttackReleased);
		EnhancedInputComponent->BindAction(SecondaryAttackAction, ETriggerEvent::Completed, this, &APlayerCharacter::OnAttackReleased);
	}
}

void APlayerCharacter::ChangeAttackMaterials()
{
	GetMesh()->SetMaterialByName(FName("Baston_1"), AttackMaterial1);
	GetMesh()->SetMaterialByName(FName("Baston_2"), AttackMaterial2);
	GetMesh()->SetMaterialByName(FName("Baston_3"), AttackMaterial3);
}

void APlayerCharacter::RevertMaterials()
{
	GetMesh()->SetMaterialByName(FName("Baston_1"), InvisibleMaterial);
	GetMesh()->SetMaterialByName(FName("Baston_2"), InvisibleMaterial);
	GetMesh()->SetMaterialByName(FName("Baston_3"), InvisibleMaterial);
}
void APlayerCharacter::ResetChangeCooldown() {
	CooldownChange = false;
}

void APlayerCharacter::ChangeGamepadLeft() {
	if (!bChangeBlock) {
		if (CurrentRMBAction == ERMBAction::LightningAttack) {
			ChangeVital();
			return;
		}
		if (CurrentRMBAction == ERMBAction::GravityAttack) {
			ChangeLightning();
			return;
		}
		if (CurrentRMBAction == ERMBAction::VitalAttack) {
			ChangeGravity();
			return;
		}
	}
}

void APlayerCharacter::ChangeGamepadRight() {
	if (!bChangeBlock) {
		if (CurrentRMBAction == ERMBAction::LightningAttack) {
			ChangeGravity();
			return;
		}
		if (CurrentRMBAction == ERMBAction::GravityAttack) {
			ChangeVital();
			return;
		}
		if (CurrentRMBAction == ERMBAction::VitalAttack) {
			ChangeLightning();
			return;
		}
	}
}

void APlayerCharacter::ChangeLightning()
{
	if (!CooldownChange && CurrentRMBAction != ERMBAction::LightningAttack) {
		if (GetWorld()->GetTimerManager().IsTimerActive(ChangeAbility))
		{
			GetWorld()->GetTimerManager().ClearTimer(ChangeAbility);
		}
		GetWorld()->GetTimerManager().SetTimer(ChangeAbility, this, &APlayerCharacter::ResetChangeCooldown, 0.2, false);
		CooldownChange = true;
		if (!bChangeBlock) {
			CurrentRMBAction = ERMBAction::LightningAttack;
			if (bGravityUnlocked && !bVitalUnlocked) ChangeToLightning();
			if (bGravityUnlocked && bVitalUnlocked) ChangeToLightning2();

			for (AActor* Objeto : GravitationalObjects)
			{
				RemoveOutline(Objeto);
			}
		}
	}
}

void APlayerCharacter::ChangeGravity() 
{
	if (!CooldownChange && CurrentRMBAction != ERMBAction::GravityAttack) {
		if (GetWorld()->GetTimerManager().IsTimerActive(ChangeAbility))
		{
			GetWorld()->GetTimerManager().ClearTimer(ChangeAbility);
		}
		GetWorld()->GetTimerManager().SetTimer(ChangeAbility, this, &APlayerCharacter::ResetChangeCooldown, 0.2, false);
		CooldownChange = true;
		if (!bChangeBlock && bGravityUnlocked) {
			CurrentRMBAction = ERMBAction::GravityAttack;
			if (!bVitalUnlocked) ChangeToGravity();
			else ChangeToGravity2();
			for (AActor* Objeto : GravitationalObjects)
			{
				AddOutline(Objeto);
			}
		}
	}
}

void APlayerCharacter::ChangeVital() 
{
	if (!CooldownChange && CurrentRMBAction != ERMBAction::VitalAttack) {
		if (GetWorld()->GetTimerManager().IsTimerActive(ChangeAbility))
		{
			GetWorld()->GetTimerManager().ClearTimer(ChangeAbility);
		}
		GetWorld()->GetTimerManager().SetTimer(ChangeAbility, this, &APlayerCharacter::ResetChangeCooldown, 0.2, false);
		CooldownChange = true;
		if (!bChangeBlock) {
			if (bVitalUnlocked) {
				CurrentRMBAction = ERMBAction::VitalAttack;
				ChangeToVital();
				for (AActor* Objeto : GravitationalObjects)
				{
					RemoveOutline(Objeto);
				}
			}
		}
	}
}

void APlayerCharacter::Dash()
{
	if (ActionState == EActionState::EAS_EquippingWeapon || ActionState == EActionState::EAS_Dead) {
		return;
	}
	if (IsOccupied() && !bCanDash || !HasEnoughMana() || !GetCharacterMovement()->IsMovingOnGround()) {
		NoManaAnim();
		bDashInputBuffered = true;
		LastInputTimeDash = GetWorld()->GetTimeSeconds();
		return;
	}
	if (TimerHandle_Healing.IsValid()) {
		GetWorldTimerManager().ClearTimer(TimerHandle_Healing);
		StopAnimMontage();
		bCargaVida = false;
		bCanAttack = true;
		bCantGethit = false;
		AttackEnd();
	}

	if (IsAbilityUnlocked(TEXT("DashShield")))
	{
		if (bCanActivateShield && !bShieldActive ) {
			ActivateShield();
		}
	}

	if (bIsCharging) {
		bAttackInputBuffered = false;
		bIsCharging = false;
		FinAim();
	}

	if (bShouldMoveForward) bShouldMoveForward = false;
	bCantGethit = false;
	bcanRotate = true;
	ActionState = EActionState::EAS_Melee;
	bIsDashing = true;
	Attributes->UseMana(DodgeCost);
	PlayerOverlay->SetManaEnergyBarPercent(Attributes->GetManaPercent());
	RevertMaterials();
	ComboEnd();
	StopAnimMontage();
	RevertMaterials();
	PlayDodgeMontage("DodgeNormal");
}

void APlayerCharacter::DashImpulse()
{
	FVector DashDirection;
	DashDirection = GetActorForwardVector();

	DashDirection = DashDirection.GetSafeNormal();
	DashStartLocation = GetActorLocation();

	if (IsAbilityUnlocked(TEXT("DashDistancia"))) {
		DashTargetLocation = DashStartLocation + DashDirection * 1000;
	}
	else {
		DashTargetLocation = DashStartLocation + DashDirection * DashDistance;
	}

	FHitResult SweepResult;
	FVector SweepStart = DashStartLocation;
	FVector SweepEnd = DashTargetLocation;

	FCollisionShape CollisionShape = FCollisionShape::MakeCapsule(GetCapsuleComponent()->GetScaledCapsuleRadius(), GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	bool bWillCollide = GetWorld()->SweepSingleByChannel(SweepResult, SweepStart, SweepEnd, FQuat::Identity, ECC_GameTraceChannel2, CollisionShape);

	if (bWillCollide && SweepResult.bBlockingHit)
	{
		DashTargetLocation = SweepResult.Location;
		DashDistance = (SweepResult.Location - DashStartLocation).Size();
	}

	SetActorRotation(DashDirection.Rotation());

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	LastValidLocation = GetActorLocation();

	DashTimeline->PlayFromStart();
}

void APlayerCharacter::CancelDash()
{
	if (DashTimeline != nullptr)
	{
		DashTimeline->Stop();
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

		FHitResult Hit;
		bool bHit = SetActorLocation(GetActorLocation(), true, &Hit);

		if (bHit && Hit.GetActor() && Hit.GetActor()->IsA<APawn>())
		{
			SetActorLocation(LastValidLocation);
		}

		DashTimeline->Stop();
	}
	bIsDashing = false;
	bCanDash = true;
	ActionState = EActionState::EAS_Unoccupied;
}

void APlayerCharacter::OnAttackReleased()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(TimerHandle_ChargeAttack))
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_ChargeAttack);

		if (bCanAttack || ActionState == EActionState::EAS_Melee) { PerformRegularAttack(); }
	}
	else if(bIsCharging && AttackType != ECurrentAttackType::None)
	{
		if (bIsAlreadyOnAttack) {
			return;
		}
		ReleasedChargedAttack();
	}
	if (bIsCharging) {
		bIsCharging = false;
	}
	if (bNoMana) {
		bAttackInputBuffered = false;
		bCanAttack = true;
	}
	//GetWorld()->GetTimerManager().ClearTimer(TimerHandle_ChargeAttack);
}

void APlayerCharacter::PerformRegularAttack() 
{
	if (bIsAttackOnCooldown) {
		return;
	}
	bNoMana = false;
	bCanAttack = false;
	bAttackInputBuffered = false;
	switch (AttackType)
	{
	case ECurrentAttackType::Melee:
		Super::Attack();
		bcanRotate = true;
		SelectAttackMontageSection(MeleeMontage);
		ActionState = EActionState::EAS_Melee;
		break;
	case ECurrentAttackType::Lightning:
		if (!(Attributes->GetMana() > LightningCost)) {
			if (bCanAttack) RevertMaterials();
			bNoMana = true; bCanAttack = true; NoManaAnim(); bAttackInputBuffered = false; break; }
		bcanRotate = true;
		SelectAttackMontageSection(LightningMontage);
		Attributes->UseMana(LightningCost);
		PlayerOverlay->SetManaEnergyBarPercent(Attributes->GetManaPercent());
		ActionState = EActionState::EAS_Melee;
		break;
	case ECurrentAttackType::Gravity:
		if (!(Attributes->GetMana() > GravityCost)) {
			if (bCanAttack) RevertMaterials();  
			bNoMana = true; 
			bCanAttack = true; 
			NoManaAnim(); 
			bAttackInputBuffered = false; 
			break; }
		ActionState = EActionState::EAS_Melee;
		if (bIsGravityCharged)
		{
			FindAndSetClosestEnemyInSight();
			if (NearestGravitationalObject && (AimTarget || CombatTarget)){
				if (IsAbilityUnlocked("GravityVelocity")) {
					PlayMontageSection(GravityMontage, "ReleaseMejorado");
				}
				else {
					PlayMontageSection(GravityMontage, "Release");
				}
				Attributes->UseMana(GravityCost);
				PlayerOverlay->SetManaEnergyBarPercent(Attributes->GetManaPercent());
				if (AimTarget) ThrowGravitationalObjects(AimTarget);
				else {
					ThrowGravitationalObjects(CombatTarget);
				}
				bIsGravityCharged = false;
				break;
			}else{
				if (IsAbilityUnlocked("GravityVelocity")) {
					PlayMontageSection(GravityMontage, "ReleaseMejorado");
				}
				else {
					PlayMontageSection(GravityMontage, "Release");
				}
				Attributes->UseMana(GravityCost);
				PlayerOverlay->SetManaEnergyBarPercent(Attributes->GetManaPercent());
				ThrowGravitationalObjectsWithoutEnemy();
				bIsGravityCharged = false;
				break;
			}
		}else
		{
			if (IsAbilityUnlocked("GravityVelocity")) {
				PlayMontageSection(GravityMontage, "GravedadMejorada");
			}
			else {
				PlayMontageSection(GravityMontage, "Attack_1");
			}
			Attributes->UseMana(GravityCost);
			PlayerOverlay->SetManaEnergyBarPercent(Attributes->GetManaPercent());
			SelectNearestGravitationalObject();
		}
		break;
	case ECurrentAttackType::Vital:
		if (bVitalActivated) {
			PlayMontageSection(VitalMontage, "Base");
			ActionState = EActionState::EAS_Melee;
			DeactivateVitalAttackEffects();
			break;
		}
		else if (!(Attributes->GetMana() > VitalCost)) { 
			if(bCanAttack) RevertMaterials();
			bCanAttack = true; 
			NoManaAnim(); 
			bAttackInputBuffered = false;
			bNoMana = true;
			break; 
		}
		PlayMontageSection(VitalMontage, "Base");
		Attributes->UseMana(VitalCost);
		PlayerOverlay->SetManaEnergyBarPercent(Attributes->GetManaPercent());
		ActionState = EActionState::EAS_Melee;
		ActivateVitalAttackEffects();
		break;
	default:
		break;
	}
	bIsAttackOnCooldown = true;
	GetWorld()->GetTimerManager().SetTimer(CooldownTimerHandle, this, &APlayerCharacter::ResetAttackCooldown, 0.3, false);

}
void APlayerCharacter::ResetAttackCooldown()
{
	bIsAttackOnCooldown = false;
}

void APlayerCharacter::ReleasedChargedAttack() 
{
	switch (AttackType)
	{
	case ECurrentAttackType::Melee:
		PlayMontageSection(MeleeMontage, "Release");
		ActionState = EActionState::EAS_Melee;
		break;
	case ECurrentAttackType::Fire:
		PlayMontageSection(FireMontage, "Release");
		break;
	case ECurrentAttackType::Lightning:
		if (IsAbilityUnlocked("RayoCargado"))
		{
			PlayMontageSection(LightningMontage, "Release");
		}
		break;
	case ECurrentAttackType::Gravity:
		break;
	case ECurrentAttackType::Vital:
		if (IsAbilityUnlocked("VitalCargado"))
		{
			if (TimerHandle_Healing.IsValid()) {
				GetWorldTimerManager().ClearTimer(TimerHandle_Healing);
			}
			if (ChargeAudioComponent)
			{
				ChargeAudioComponent->Stop();
			}
			bAttackInputBuffered = false;
			AttackEnd();
			StopAnimMontage();
			RemoveChargedVital();
			bCargaVida = false;
			bcanRotate = false;
			bCanAttack = true;
			bCantGethit = false;
		}
		break;
	default:
		break;
	}
}

void APlayerCharacter::AddImpulseMelee() {
	ChargeAttackInitialPosition = GetActorLocation();
	ChargeAttackTargetPosition = ChargeAttackInitialPosition + GetActorForwardVector() * ImpulseForce;
	ChargeAttackTimeline->PlayFromStart();
}

void APlayerCharacter::ExecuteChargedAttack()
{
	switch (AttackType)
	{
	case ECurrentAttackType::Melee:
		Super::Attack();
		bcanRotate = true;
		bCanAttack = false;
		PlayMontageSection(MeleeMontage, "Charged");
		ActionState = EActionState::EAS_Melee;
		bCantGethit = true;
		MeleeChargedDamage = MeleeChargedDamageInicial;
		ImpulseForce = 350;
		break;
	case ECurrentAttackType::Lightning:
		if (IsAbilityUnlocked("RayoCargado"))
		{
			Super::Attack();
			if (Attributes->GetMana() < 20.f) { NoManaAnim(); return; }
			Attributes->UseMana(20.0f);
			bAttackInputBuffered = false;
			bCargaRayo = true;
			PlayerOverlay->SetManaBarPercent(Attributes->GetManaPercent());
			PlayerOverlay->SetManaEnergyBarPercent(Attributes->GetManaPercent());
			AddVFXLightningCharge();
			bcanRotate = true;
			bCanAttack = false;
			PlayMontageSection(LightningMontage, "Charged");
			ActionState = EActionState::EAS_Melee;
			bCantGethit = true;
			if (ElectricChargeSoundCue && ChargeAudioComponent)
			{
				ChargeAudioComponent->SetSound(ElectricChargeSoundCue);
				ChargeAudioComponent->Play();
			}
		}
		else
		{
			PerformRegularAttack();
		}
		break;
	case ECurrentAttackType::Vital:
		if (IsAbilityUnlocked("VitalCargado")) {
			DeactivateVitalAttackEffects();
			Super::Attack();
			bCargaVida = true;
			bcanRotate = true;
			bCanAttack = false;
			GetWorldTimerManager().SetTimer(TimerHandle_Healing, this, &APlayerCharacter::HandleHealing, 1.0f, true);
			AddChargedVital();
			ActionState = EActionState::EAS_Melee;
			PlayMontageSection(VitalMontage, "Charge");
			bCantGethit = true;
			if (VitalChargeSoundCue && ChargeAudioComponent)
			{
				ChargeAudioComponent->SetSound(VitalChargeSoundCue);
				ChargeAudioComponent->Play();
			}
		}
	default:
		break;
	}
}

void APlayerCharacter::AttackMeleeCombo()
{
	if (bHasNotReleasedAttack || ActionState == EActionState::EAS_Dead) return;
	AttackCombo(ECurrentAttackType::Melee);
}

void APlayerCharacter::AttackMagicCombo() 
{
	if (bHasNotReleasedAttack || ActionState == EActionState::EAS_Dead) return;
	switch (CurrentRMBAction)
	{
	case ERMBAction::LightningAttack:
		AttackCombo(ECurrentAttackType::Lightning);
		break;
	case ERMBAction::GravityAttack:
		if (bGravityUnlocked)
		{
			AttackCombo(ECurrentAttackType::Gravity);
		}
		break;
	case ERMBAction::VitalAttack:
		AttackCombo(ECurrentAttackType::Vital);
		break;
	}
}

void APlayerCharacter::AttackCombo(ECurrentAttackType Type)
{
	Super::Attack();
	if (ActionState == EActionState::EAS_EquippingWeapon || ActionState == EActionState::EAS_Dead) {
		return;
	}
	AttackType = Type;

	bCanDash = false;

	if (CanAttack() || bCanAttack)
	{
		StartCharging();
	}

	if (!CanAttack() && !bNoMana)
	{
		bAttackInputBuffered = true;
		LastInputTimeAttack = GetWorld()->GetTimeSeconds();
		return;
	}
}

void APlayerCharacter::ToggleAimState()
{
	if (ActionState == EActionState::EAS_Unoccupied)
	{
		ActionState = EActionState::EAS_Aiming;
	}
	else if (CharacterState == ECharacterState::ECS_EquippedMagicFire)
	{
		ActionState = EActionState::EAS_Unoccupied;
	}
}

float APlayerCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDashing || ActionState == EActionState::EAS_Dead) {
		return 0.0f;
	}

	if (bShieldActive)
	{
		GetWorldTimerManager().SetTimer(CooldownShield, this, &APlayerCharacter::CanActivateShield, 8.0f, false);
		bShieldActive = false;
		DeactivateShieldVFX();
		HandleDamage(DamageAmount / 2);
		PlayerOverlay->SetHealthEnergyBarPercent(Attributes->GetHealthPercent());
		SetHUDHealth();
		SpawnHitParticles();
		if (ShieldBreakSoundCue && CharacterAudioComponent)
		{
			CharacterAudioComponent->SetSound(ShieldBreakSoundCue);
			CharacterAudioComponent->Play();
		}

		return DamageAmount / 2;
	}

	if (bVitalActivated) {
		if (IsAbilityUnlocked("VitalUpgrade")) {
			HandleDamage(DamageAmount - DamageAmount * 0.4);
			PlayerOverlay->SetHealthEnergyBarPercent(Attributes->GetHealthPercent());
			SetHUDHealth();
			SpawnHitParticles();
			return DamageAmount - DamageAmount * 0.4;
		}
		else {
			HandleDamage(DamageAmount - DamageAmount * 0.2);
			PlayerOverlay->SetHealthEnergyBarPercent(Attributes->GetHealthPercent());
			SetHUDHealth();
			SpawnHitParticles();
			return DamageAmount - DamageAmount * 0.2;
		}
	}

	HandleDamage(DamageAmount);
	PlayerOverlay->SetHealthEnergyBarPercent(Attributes->GetHealthPercent());
	SetHUDHealth();
	SpawnHitParticles();
	return DamageAmount;
}

void APlayerCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	if (bIsDashing || ActionState == EActionState::EAS_Dead || bCantGethit) {
		return;
	}
	if (!bIsInvincible) {
		bCanAttack = false;
		bCanDash = false;
		StopAnimMontage();
		OnWeaponColliderDeactivated();
		LightningChargedCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (Attributes && Attributes->GetHealthPercent() > 0.f)
		{
			ActionState = EActionState::EAS_HitReaction;
		}
		if (Hitter->ActorHasTag("Boss")) {
			PushBossInteraction(Hitter);
		}
		bShouldMoveForward = false;
		bShouldOrientTowardsTarget = false;
		bAttackInputBuffered = false;
		if (GetWorld()->GetTimerManager().IsTimerActive(TimerHandle_ChargeAttack))
		{
			GetWorld()->GetTimerManager().ClearTimer(TimerHandle_ChargeAttack);
		}
		bIsCharging = false;
		bHasNotReleasedAttack = false;
		FinAim();
		RevertMaterials();
	}

	Super::GetHit_Implementation(ImpactPoint, Hitter);
}

void APlayerCharacter::SetOverlappingItem(AItem* Item)
{
	OverlappingItem = Item;
}

void APlayerCharacter::AddSoulsInterface(int32 Souls)
{
	if (Attributes && PlayerOverlay)
	{
		PlayerOverlay->SetSouls(Souls);
	}
}

void APlayerCharacter::AddSouls(ASoul* Soul)
{
	if (Attributes && PlayerOverlay) 
	{
		Attributes->AddSouls(Soul->GetSouls());
		PlayerOverlay->SetSouls(Attributes->GetSouls());
	}
}

void APlayerCharacter::AddGold(int32 GoldAdded)
{
	if (Attributes && PlayerOverlay)
	{
		Attributes->AddGold(GoldAdded);
		PlayerOverlay->SetGold(Attributes->GetGold());
	}
}

void APlayerCharacter::DashTick(float Value)
{
	FVector NewLocation = FMath::Lerp(DashStartLocation, DashTargetLocation, Value);
	FHitResult Hit;

	bool bHit = SetActorLocation(NewLocation, true, &Hit);

	if (!bHit || (Hit.GetActor() && Hit.GetActor()->IsA<APawn>() == false))
	{
		LastValidLocation = NewLocation;
	}
}

void APlayerCharacter::SetAttackToNone()
{
	AttackType = ECurrentAttackType::None;
}

void APlayerCharacter::FinAim_Implementation()
{
}

void APlayerCharacter::MoveForward(float ScaleValue)
{
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(Direction, ScaleValue);
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	Tags.Add(FName("EngageableTarget"));
	InitializePlayerOverlay();
	DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter::OnSphereOverlap);
	DetectionSphere->OnComponentEndOverlap.AddDynamic(this, &APlayerCharacter::OnSphereEndOverlap);
	WeaponCollider->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter::OnWeaponColliderOverlap);
	LightningChargedCollider->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter::OnWeaponColliderOverlap);
	ConfigureChargeAttackTimeline();
	InitializeDashTimeline();
	ChangeLightning();

	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(this);

	bool bIsOverlapping = GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		DetectionSphere->GetComponentLocation(),
		FQuat::Identity,
		DetectionSphere->GetCollisionObjectType(),
		FCollisionShape::MakeSphere(DetectionSphere->GetScaledSphereRadius()),
		CollisionQueryParams
	);

	if (bIsOverlapping)
	{
		for (const FOverlapResult& OverlapResult : OverlapResults)
		{
			AActor* OtherActor = OverlapResult.GetActor();
			if (OtherActor && OtherActor != this) 
			{
				if (OtherActor->ActorHasTag(FName("Enemy")) || OtherActor->ActorHasTag(FName("Bateria")))
				{
					EnemiesInRange.AddUnique(OtherActor);
				}

				if (OtherActor->ActorHasTag(FName("Gravedad")))
				{
					GravitationalObjects.AddUnique(OtherActor);
					if (CurrentRMBAction == ERMBAction::GravityAttack)
					{
						AddOutline(OtherActor);
					}
					UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(OtherActor->GetRootComponent());
					if (PrimitiveComponent)
					{
						PrimitiveComponent->SetEnableGravity(false);
					}
				}
			}
		}
	}
}

void APlayerCharacter::SwitchTargetAxis(const FInputActionValue& AxisValue)
{
	const FVector2D MovementVector = AxisValue.Get<FVector2D>();
	//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, FString::Printf(TEXT("X: %f, Y: %f"), MovementVector.X, MovementVector.Y));

	ETargetSwitchDirection SwitchDirection = (MovementVector.X > 0) ? ETargetSwitchDirection::Right : ETargetSwitchDirection::Left;
	ChangeTargetByDirection(SwitchDirection);
}

void APlayerCharacter::EnableTrigger()
{
	bCanTrigger = true;
}

void APlayerCharacter::ChangeTargetByDirection(ETargetSwitchDirection SwitchDirection)
{
	bool bSwitchLeft = (SwitchDirection == ETargetSwitchDirection::Left);

	LockOn(bSwitchLeft);
}

void APlayerCharacter::LockOn(bool bSwitchLeft)
{
	if (EnemiesInRange.Num() > 0 && AimTarget)
	{
		const FVector CurrentTargetLocation = AimTarget->GetActorLocation();
		AActor* BestTarget = nullptr;
		float MinDistance = FLT_MAX;

		FVector ForwardVector = ViewCamera->GetForwardVector();
		const FVector MyLocation = GetActorLocation();

		for (AActor* Enemy : EnemiesInRange)
		{
			if (Enemy != AimTarget)
			{
				FVector EnemyLocation = Enemy->GetActorLocation();
				FVector DirectionToEnemy = (EnemyLocation - CurrentTargetLocation).GetSafeNormal();
				FVector CrossProduct = FVector::CrossProduct(ForwardVector, DirectionToEnemy);
				bool IsLeft = CrossProduct.Z > 0;

				if ((bSwitchLeft && IsLeft) || (!bSwitchLeft && !IsLeft))
				{
					float Distance = (EnemyLocation - CurrentTargetLocation).Size();

					FHitResult HitResult;
					FCollisionQueryParams CollisionParams;
					CollisionParams.AddIgnoredActor(this);  
					CollisionParams.bTraceComplex = true;

					bool bHit = GetWorld()->LineTraceSingleByChannel(
						HitResult,
						MyLocation,
						EnemyLocation,
						ECC_GameTraceChannel3,
						CollisionParams
					);

					if (!bHit || HitResult.GetActor() == Enemy)
					{
						if (Distance < MinDistance)
						{
							MinDistance = Distance;
							BestTarget = Enemy;
						}
					}
				}
			}
		}

		if (BestTarget)
		{
			AimTarget = BestTarget;
			if (ACharacter* Character = Cast<ACharacter>(AimTarget))
			{
				UCapsuleComponent* EnemyCapsuleComponent = Character->GetCapsuleComponent();
				float CapsuleHalfHeight = EnemyCapsuleComponent->GetUnscaledCapsuleHalfHeight();
				FVector ComponentScale = EnemyCapsuleComponent->GetComponentScale();

				float ScaledFullHeight = CapsuleHalfHeight * ComponentScale.Z;
				WidgetHeightAboveActor = ScaledFullHeight * HeightAboveEnemy;
			}
		}
	}
}

void APlayerCharacter::LockOn()
{
	if (EnemiesInRange.Num() > 0)
	{
		if (!AimTarget || !EnemiesInRange.Contains(AimTarget))
		{
			float ClosestEnemyDistance = FLT_MAX;
			AActor* ClosestEnemy = nullptr;
			const FVector MyLocation = GetActorLocation();

			for (AActor* Enemy : EnemiesInRange)
			{
				FVector EnemyLocation = Enemy->GetActorLocation();
				float DistanceToEnemy = FVector::Distance(MyLocation, EnemyLocation);

				FHitResult HitResult;
				FCollisionQueryParams CollisionParams;
				CollisionParams.AddIgnoredActor(this);  
				CollisionParams.bTraceComplex = true;

				bool bHit = GetWorld()->LineTraceSingleByChannel(
					HitResult,
					MyLocation,
					EnemyLocation,
					ECC_GameTraceChannel3,
					CollisionParams
				);

				if (!bHit || HitResult.GetActor() == Enemy)
				{
					if (DistanceToEnemy < ClosestEnemyDistance)
					{
						ClosestEnemy = Enemy;
						ClosestEnemyDistance = DistanceToEnemy;
					}
				}
			}

			AimTarget = ClosestEnemy;
		}
		else
		{
			if (bVitalActivated) {
				RunState = ERunState::ERS_Fast;
			}
			else {
				RunState = ERunState::ERS_Normal;
			}
			AimTarget = nullptr;
			TargetSelectionWidget->SetVisibility(false);
			bIsWidgetAnimationPlaying = false;
			//bUseControllerRotationYaw = false;
		}

		if (AimTarget)
		{
			RunState = ERunState::ERS_Aggro;
			//bUseControllerRotationYaw = true;
			CameraBoom->SetRelativeLocation(FVector(0, 35, 70));
		}
	}
}

void APlayerCharacter::AddOutline(AActor* OtherActor) {
	if (!OtherActor)
	{
		return;
	}

	UGeometryCollectionComponent* GeometryComp = Cast<UGeometryCollectionComponent>(OtherActor->GetComponentByClass(UGeometryCollectionComponent::StaticClass()));

	if (GeometryComp)
	{
		GeometryComp->SetRenderCustomDepth(true); 
	}
}

void APlayerCharacter::RemoveOutline(AActor* OtherActor) {
	if (!OtherActor)
	{
		return;
	}

	UGeometryCollectionComponent* GeometryComp = Cast<UGeometryCollectionComponent>(OtherActor->GetComponentByClass(UGeometryCollectionComponent::StaticClass()));

	if (GeometryComp)
	{
		GeometryComp->SetRenderCustomDepth(false);
	}
}

void APlayerCharacter::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->ActorHasTag(FName("Enemy")) || OtherActor->ActorHasTag(FName("Bateria")))
	{
		EnemiesInRange.AddUnique(OtherActor);
	}

	if (OtherActor->ActorHasTag(FName("Gravedad"))) {
		GravitationalObjects.AddUnique(OtherActor);
		if (CurrentRMBAction == ERMBAction::GravityAttack) {
			AddOutline(OtherActor);
		}
		UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(OtherActor->GetRootComponent());
		if (PrimitiveComponent)
		{
			PrimitiveComponent->SetEnableGravity(false);
		}
	}
}

void APlayerCharacter::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (GravitationalObjects.Contains(OtherActor))
	{
		GravitationalObjects.Remove(OtherActor);
		if (CurrentRMBAction == ERMBAction::GravityAttack) {
			RemoveOutline(OtherActor);
		}
	}

	if (EnemiesInRange.Contains(OtherActor))
	{
		EnemiesInRange.Remove(OtherActor);
	}

	if (AimTarget == OtherActor)
	{
		AimTarget = nullptr;
		TargetSelectionWidget->SetVisibility(false);
		bIsWidgetAnimationPlaying = false;
		if (bVitalActivated) {
			RunState = ERunState::ERS_Fast;
		}
		else {
			RunState = ERunState::ERS_Normal;
		}
	}

	if (CombatTarget == OtherActor)
	{
		CombatTarget = nullptr;
	}
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	if (ActionState == EActionState::EAS_EquippingWeapon || ActionState == EActionState::EAS_Dead || bIsInvincible) {
		return;
	}
	const FVector2D MovementVector = Value.Get<FVector2D>();
	if (!bCanDash && !MovementVector.IsNearlyZero() && ActionState != EActionState::EAS_Melee)
	{
		StopAnimMontage();
		CancelDash();
	}
	const FRotator CameraRotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.f, CameraRotation.Yaw, 0.f);


	if (ActionState == EActionState::EAS_Unoccupied && !bcanRotate)
	{
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(ForwardDirection, MovementVector.Y);

		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(RightDirection, MovementVector.X);

		Direccion = MovementVector;
	}

	if (bcanRotate && !MovementVector.IsNearlyZero() || ActionState == EActionState::EAS_Unoccupied)
	{
		const FVector WorldMovementDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X) * MovementVector.Y +
			FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y) * MovementVector.X;

		FRotator DesiredRotation = FRotationMatrix::MakeFromX(WorldMovementDirection).Rotator();

		float DeltaTime = GetWorld()->GetDeltaSeconds();
		float RotationSpeed = 10.0f;
		FRotator SmoothRotation = FMath::RInterpTo(GetActorRotation(), DesiredRotation, DeltaTime, RotationSpeed);

		SetActorRotation(SmoothRotation);
	}
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerPitchInput(LookAxisVector.Y);
	AddControllerYawInput(LookAxisVector.X);
}

void APlayerCharacter::StartCharging()
{
	bIsCharging = true;
	UE_LOG(LogTemp, Warning, TEXT("Charging Started"));
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_ChargeAttack, this, &APlayerCharacter::ExecuteChargedAttack, ChargeAttackThreshold, false);
}

void APlayerCharacter::StopCharging()
{
	bIsCharging = false;
}

void APlayerCharacter::Attack()
{
	Super::Attack();

	FindAndSetClosestEnemyInSight();

	if (CanAttack() || bCanAttack)
	{
		bCanAttack = false;
		SelectAttackMontageSection(MeleeMontage);
		ActionState = EActionState::EAS_Melee;
	}
}

void APlayerCharacter::FindAndSetClosestEnemyInSight()
{
	if (EnemiesInRange.Num() == 0) return;

	AActor* ClosestEnemy = nullptr;
	float ClosestDistanceSq = FLT_MAX;
	FVector MyLocation = GetActorLocation();
	FVector ForwardVector = GetActorForwardVector();

	if (AimTarget) {
		ClosestEnemy = AimTarget;
		bShouldOrientTowardsTarget = true;
		return;
	}

	for (AActor* Enemy : EnemiesInRange)
	{
		if (Enemy->ActorHasTag("Enemy")) 
		{
			FVector EnemyLocation = Enemy->GetActorLocation();
			FVector DirectionToEnemy = Enemy->GetActorLocation() - MyLocation;
			float DistanceSq = DirectionToEnemy.SizeSquared();
			DirectionToEnemy.Normalize();

			float DotProduct = FVector::DotProduct(ForwardVector, DirectionToEnemy);
			float Angle = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

			if (Angle <= 130.0f)
			{
				FHitResult HitResult;
				FCollisionQueryParams CollisionParams;
				CollisionParams.AddIgnoredActor(this);  
				CollisionParams.bTraceComplex = true;

				bool bHit = GetWorld()->LineTraceSingleByChannel(
					HitResult,
					MyLocation,
					EnemyLocation,
					ECC_GameTraceChannel3,
					CollisionParams
				);

				if (!bHit || HitResult.GetActor() == Enemy)
				{
					if (DistanceSq < ClosestDistanceSq)
					{
						ClosestDistanceSq = DistanceSq;
						ClosestEnemy = Enemy;
					}
				}
			}
			else {
				ClosestEnemy = nullptr;
			}
		}
	}

	CombatTarget = ClosestEnemy;
	if (CombatTarget) 
	{
		bShouldOrientTowardsTarget = true;
	}
}

void APlayerCharacter::OrientTowards(AActor* Target)
{
	if (Target)
	{
		FVector TargetLocation = Target->GetActorLocation();
		FVector MyLocation = GetActorLocation();
		FRotator CurrentRotation = GetActorRotation();

		FRotator TargetRotation = (TargetLocation - MyLocation).Rotation();
		TargetRotation.Pitch = CurrentRotation.Pitch;
		TargetRotation.Roll = CurrentRotation.Roll;

		float DeltaTime = GetWorld()->GetDeltaSeconds();
		float InterpSpeed = 10.0f;
		FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, InterpSpeed);

		SetActorRotation(NewRotation);
	}
}
//Cambiar estasFunciones
void APlayerCharacter::AttackFire()
{
	if (!HasEnoughMana()) return;
	Super::Attack();
	if (CanAttack())
	{
		SelectAttackMontageSection(FireMontage);
		Attributes->UseMana(Attributes->GetDodgeCost());
		ActionState = EActionState::EAS_Melee;
	}
}

void APlayerCharacter::AttackGravity()
{
	if (!HasEnoughMana()) return;
	Super::Attack();
	if (CanAttack())
	{
		SelectAttackMontageSection(GravityMontage);
		Attributes->UseMana(Attributes->GetDodgeCost());
		ActionState = EActionState::EAS_Melee;
	}
}

void APlayerCharacter::AttackLightning()
{
	if (!HasEnoughMana()) return;
	Super::Attack();
	if (CanAttack())
	{
		SelectAttackMontageSection(LightningMontage);
		Attributes->UseMana(Attributes->GetDodgeCost());
		ActionState = EActionState::EAS_Melee;
	}
}

void APlayerCharacter::Lock()
{
	LockOn();
}

void APlayerCharacter::AttackEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
	StopAnimMontage();
}

void APlayerCharacter::DodgeEnd()
{
	Super::DodgeEnd();

	ActionState = EActionState::EAS_Unoccupied;
}

bool APlayerCharacter::CanAttack()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState != ECharacterState::ECS_Unequipped;
}

void APlayerCharacter::AnimAttackEnd()
{
	bCanAttack = true;
}

bool APlayerCharacter::CanDisarm()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState != ECharacterState::ECS_Unequipped;
}

bool APlayerCharacter::CanArm()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState == ECharacterState::ECS_Unequipped &&
		EquippedWeapon;
}

void APlayerCharacter::Disarm()
{
	PlayEquipMontage(FName("Unequip"));
	CharacterState = ECharacterState::ECS_Unequipped;
	ActionState = EActionState::EAS_EquippingWeapon;
}

void APlayerCharacter::Arm()
{
	PlayEquipMontage(FName("Equip"));
	CharacterState = ECharacterState::ECS_EquippedMagicFire;
	ActionState = EActionState::EAS_EquippingWeapon;
}

void APlayerCharacter::PlayEquipMontage(const FName SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EquipMontage)
	{
		AnimInstance->Montage_Play(EquipMontage);
		AnimInstance->Montage_JumpToSection(SectionName, EquipMontage);
	}
}

void APlayerCharacter::Die()
{
	Super::Die();

	ActionState = EActionState::EAS_Dead;
	DisableMeshCollision();
	PlayMontageSection(DeathMontage, "Death1");
}

bool APlayerCharacter::HasEnoughMana()
{
	return Attributes && Attributes->GetMana() > Attributes->GetDodgeCost();
}

bool APlayerCharacter::IsOccupied()
{
	return ActionState != EActionState::EAS_Unoccupied;
}

void APlayerCharacter::AttachWeaponToBack()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("SpineSocket"));
	}
}

void APlayerCharacter::AttachWeaponToHand()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("RightHandSocket"));
	}
}

void APlayerCharacter::FinishEquipping()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void APlayerCharacter::HitReactEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
	bCanDash = true;
	bCanAttack = true;
}

void APlayerCharacter::InitializeDashTimeline()
{
	if (DashCurve)
	{
		FOnTimelineFloat TimelineProgress;
		TimelineProgress.BindUFunction(this, FName("DashTick"));
		DashTimeline->AddInterpFloat(DashCurve, TimelineProgress);
	}
}

bool APlayerCharacter::IsUnoccupied()
{
	return ActionState == EActionState::EAS_Unoccupied;
}

void APlayerCharacter::InitializePlayerOverlay()
{
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		APlayerHUD* PlayerHUD = Cast<APlayerHUD>(PlayerController->GetHUD());
		if (PlayerHUD)
		{
			PlayerOverlay = PlayerHUD->GetPlayerOverlay();
			if (PlayerOverlay && Attributes)
			{
				PlayerOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
				PlayerOverlay->SetManaBarPercent(1.f);
				PlayerOverlay->SetGold(0);
				PlayerOverlay->SetSouls(0);
			}
		}

		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(SlashContext, 0);
		}
	}
}


void APlayerCharacter::SetHUDMana()
{
	if (Attributes && PlayerOverlay)
	{
		PlayerOverlay->SetManaBarPercent(Attributes->GetManaPercent());
	}
}

void APlayerCharacter::SetHUDHealth()
{
	if (PlayerOverlay && Attributes)
	{
		PlayerOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
	}
}
void APlayerCharacter::SelectNearestGravitationalObject()
{
	AActor* ClosestObject = nullptr;
	float MinDistance = FLT_MAX;
	FVector MyLocation = GetActorLocation();

	for (AActor* Object : GravitationalObjects)
	{
		if (!Object) continue; // Asegura que el objeto sea válido
		float Distance = FVector::Dist(MyLocation, Object->GetActorLocation());
		if (Distance < MinDistance)
		{
			MinDistance = Distance;
			ClosestObject = Object;
		}
	}

	NearestGravitationalObject = ClosestObject;

	if (NearestGravitationalObject != nullptr) {
		NearestGravitationalObject->Tags.Remove("Gravedad");
		FVector ObjectLocation = NearestGravitationalObject->GetActorLocation();
		FHitResult HitResult;
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(this);

		bool bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			MyLocation,
			ObjectLocation,
			ECC_GameTraceChannel3,
			CollisionParams
		);

		if (!bHit)
		{
			TArray<UActorComponent*> Components;
			NearestGravitationalObject->GetComponents<UActorComponent>(Components);
			for (UActorComponent* Component : Components)
			{
				UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(Component);
				if (PrimitiveComponent)
				{
					PrimitiveComponent->SetCollisionResponseToChannel(ECC_Destructible, ECR_Ignore);
					PrimitiveComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
					PrimitiveComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);

					PrimitiveComponent->SetSimulatePhysics(false);
					PrimitiveComponent->SetPhysicsLinearVelocity(FVector::ZeroVector); 
				}
			}
			TArray<UActorComponent*> Components2;

			NearestGravitationalObject->GetComponents(UGeometryCollectionComponent::StaticClass(), Components2);

			for (UActorComponent* Component : Components2)
			{
				UGeometryCollectionComponent* GeometryComponent = Cast<UGeometryCollectionComponent>(Component);
				if (GeometryComponent)
				{
					GeometryComponent->SetSimulatePhysics(false);

					GeometryComponent->BodyInstance.SetCollisionEnabled(ECollisionEnabled::QueryOnly);
				}
			}
			bIsGravitationalPullActive = true;
			bIsGravityCharged = true;

			if (GrabParticleEffectTemplate)
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(
					GetWorld(),
					GrabParticleEffectTemplate,
					ObjectLocation,
					NearestGravitationalObject->GetActorRotation()
				);
			}

			if (!ActiveParticleComponent && GravitySystemTemplate)
			{
				ActiveParticleComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
					GravitySystemTemplate,
					NearestGravitationalObject->GetRootComponent(),
					NAME_None,
					FVector::ZeroVector,
					FRotator::ZeroRotator,
					EAttachLocation::KeepRelativeOffset,
					false
				);
			}
		}
		else
		{
			NearestGravitationalObject = nullptr;
			bIsGravitationalPullActive = false;
			bIsGravityCharged = false;
		}
	}
}

void APlayerCharacter::ThrowGravitationalObjectsWithoutEnemy()
{
	if (NearestGravitationalObject)
	{
		FVector ForwardDirection = GetActorForwardVector();
		ForwardDirection.Z = 0; 
		FVector ThrowDirection = ForwardDirection.GetSafeNormal();

		StartPosition = NearestGravitationalObject->GetActorLocation();
		float MaxThrowDistance = 500.0f;

		FVector EndPosition = StartPosition + ThrowDirection * MaxThrowDistance;
		FHitResult HitResult;
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(this); 
		CollisionParams.AddIgnoredActor(NearestGravitationalObject); 
		bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartPosition, EndPosition, ECC_Camera, CollisionParams);

		if (bHit && HitResult.bBlockingHit)
		{
			TargetPosition = HitResult.Location - ThrowDirection * 100.0f;
		}
		else
		{
			TargetPosition = EndPosition;
		}

		TargetPosition.Z = GetActorLocation().Z;
		

		ThrowDuration = 0.3f;
		ElapsedTime = 0.0f;

		bIsGravitationalPullActive = false;

		GetWorldTimerManager().SetTimer(ThrowTimerHandle, this, &APlayerCharacter::UpdateThrowPosition, 0.01f, true);

		if (ActiveParticleComponent)
		{
			ActiveParticleComponent->Deactivate();
			ActiveParticleComponent = nullptr;
		}
		RemoveOutline(NearestGravitationalObject);
		GravitationalObjects.Remove(NearestGravitationalObject);
	}
}

void APlayerCharacter::ThrowGravitationalObjects(AActor* Target)
{
	if (NearestGravitationalObject)
	{
		ThrowingActor = Target;
		FVector ThrowDirection = (ThrowingActor->GetActorLocation() - NearestGravitationalObject->GetActorLocation()).GetSafeNormal();
		float ThrowDistance = FVector::Distance(ThrowingActor->GetActorLocation(), NearestGravitationalObject->GetActorLocation());

		StartPosition = NearestGravitationalObject->GetActorLocation();
		TargetPosition = ThrowingActor->GetActorLocation() - FVector(0.0f, 0.0f, 100.0f);

		ThrowDuration = 0.3f; 
		ElapsedTime = 0.0f;

		bIsGravitationalPullActive = false;

		GetWorldTimerManager().SetTimer(ThrowTimerHandle, this, &APlayerCharacter::UpdateThrowPosition, 0.01f, true);

		if (ActiveParticleComponent)
		{
			ActiveParticleComponent->Deactivate();
			ActiveParticleComponent = nullptr;
		}

		GravitationalObjects.Remove(NearestGravitationalObject);
		RemoveOutline(NearestGravitationalObject);
	}
}

void APlayerCharacter::UpdateThrowPosition()
{
	if (NearestGravitationalObject)
	{
		ElapsedTime += 0.01f;

		float LerpAlpha = FMath::Clamp(ElapsedTime / ThrowDuration, 0.0f, 1.0f);
		FVector NewPosition = FMath::Lerp(StartPosition, TargetPosition, LerpAlpha);

		NearestGravitationalObject->SetActorLocation(NewPosition);

		if (ElapsedTime >= ThrowDuration)
		{
			GetWorldTimerManager().ClearTimer(ThrowTimerHandle);

			UPrimitiveComponent* RootComp = Cast<UPrimitiveComponent>(NearestGravitationalObject->GetRootComponent());
			if (RootComp)
			{
				RootComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
			}

			if (ThrowingActor && (CombatTarget || AimTarget)) {
				if (IsAbilityUnlocked(TEXT("GravityDamage"))) {
					UGameplayStatics::ApplyDamage(ThrowingActor, GravityPoweredDamage, GetInstigatorController(), this, UDamageType::StaticClass());
				}
				else {
					UGameplayStatics::ApplyDamage(ThrowingActor, GravityDamage, GetInstigatorController(), this, UDamageType::StaticClass());
				}
				CameraShake();
				bShouldOrientTowardsTarget = false;
				CombatTarget = nullptr;
			}
			AActor* SavedActor = NearestGravitationalObject;
			NearestGravitationalObject->Tags.Remove("Gravedad");
			HandleGravitationalThrowEnd(TargetPosition, SavedActor);
			NearestGravitationalObject->Destroy();

			NearestGravitationalObject = nullptr;
			bIsGravitationalPullActive = false;
			bIsGravityCharged = false;
		}
	}
}

void APlayerCharacter::DeactivateGravityAbility()
{
	bIsGravitationalPullActive = false;
}

void APlayerCharacter::UnlockAbility(FString AbilityName)
{
	for (FAbilityUnlockInfo& Ability : AbilitiesToUnlock)
	{
		if (Ability.AbilityName == AbilityName && !Ability.bIsUnlocked)
		{
			Ability.bIsUnlocked = true;
			break;
		}
	}
}

bool APlayerCharacter::CanBuyAbility(FString AbilityName) {
	for (FAbilityUnlockInfo& Ability : AbilitiesToUnlock)
	{
		if (Ability.AbilityName == AbilityName && Attributes->GetSouls() >= Ability.SoulCost)
		{
			Attributes->UseSouls(Ability.SoulCost);
			PlayerOverlay->SetSouls(Attributes->GetSouls());
			UnlockAbility(AbilityName);
			return true;
		}
		else if (Ability.AbilityName == AbilityName) {
			return false;
		}
	}
	return false;
}

bool APlayerCharacter::IsAbilityUnlocked(FString AbilityName) const
{
	for (const FAbilityUnlockInfo& Ability : AbilitiesToUnlock)
	{
		if (Ability.AbilityName == AbilityName)
		{
			return Ability.bIsUnlocked;
		}
	}
	return false;
}

void APlayerCharacter::ActivateShield()
{
	bShieldActive = true;
	bCanActivateShield = false;
	ActivateShieldVFX();
}
void APlayerCharacter::CanActivateShield() {
	bCanActivateShield = true;
}


void APlayerCharacter::ActivateVitalAttackEffects()
{
	GetWorldTimerManager().SetTimer(TimerHandle_UsingVital, this, &APlayerCharacter::HandleManaVital, 1.0f, true);
	bVitalActivated = true;
	if (!AimTarget) {
		RunState = ERunState::ERS_Fast;
	}
	RemoveVital();
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (MovementComponent)
	{
		if (IsAbilityUnlocked("VitalUpgrade")) {
			MovementComponent->MaxWalkSpeed = 960;
		}
		else {
			MovementComponent->MaxWalkSpeed = 750;
		}
	}
	AddVital();
}

void APlayerCharacter::DeactivateVitalAttackEffects()
{
	bVitalActivated = false;
	if (TimerHandle_UsingVital.IsValid()) {
		GetWorldTimerManager().ClearTimer(TimerHandle_UsingVital);
	}
	if (!AimTarget) {
		RunState = ERunState::ERS_Normal;
	}
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (MovementComponent)
	{
		MovementComponent->MaxWalkSpeed = 600; 
	}
	RemoveVital();
}

void APlayerCharacter::AddLightning() 
{
	if (LightningStrikeCount < 2) {
		if (Attributes->GetMana() < (20.f)) { NoManaAnim(); return; }
		Attributes->UseMana(20.0f);
		AddVFXLightningCharge();
		PlayerOverlay->SetManaBarPercent(Attributes->GetManaPercent());
		PlayerOverlay->SetManaEnergyBarPercent(Attributes->GetManaPercent());
		LightningStrikeCount++;
		if (ElectricChargeSoundCue && ChargeAudioComponent)
		{
			ChargeAudioComponent->SetSound(ElectricChargeSoundCue);
			ChargeAudioComponent->Play();
		}
	}
}

void APlayerCharacter::SpawnLightning()
{
	TArray<float> Angles;
	switch (LightningStrikeCount)
	{
	case 0:
		Angles.Add(0);
		break;
	case 1:
		Angles = { -30, 0, 30 };
		break;
	default:
		Angles = { -60, -30, 0, 30, 60 };
		break;
	}

	bCargaRayo = false;

	FVector CharacterLocation = GetActorLocation() + FVector(0, 0, 30);
	FVector ForwardVector = GetActorForwardVector();

	for (float Angle : Angles)
	{
		FVector Direction = ForwardVector.RotateAngleAxis(Angle, FVector::UpVector);
		FVector BeamEndLocation = CharacterLocation + (Direction * 1000.0f);

		BeamNiagara(CharacterLocation + (Direction * 50.0f), BeamEndLocation);
	}

	SpawnOrAdjustDamageCollider(Angles.Num());
	LightningChargedCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TArray<AActor*> OverlappingActors;
	LightningChargedCollider->GetOverlappingActors(OverlappingActors);

	for (AActor* Actor : OverlappingActors)
	{
		if (Actor && Actor != this)
		{
			if (IsAbilityUnlocked(TEXT("RayoDamage"))) {
				UGameplayStatics::ApplyDamage(Actor, LightningChargedPoweredDamage + (20 * LightningStrikeCount), GetInstigator()->GetController(), this, UDamageType::StaticClass());
			}
			else {
				UGameplayStatics::ApplyDamage(Actor, LightningChargedDamage + (20 * LightningStrikeCount), GetInstigator()->GetController(), this, UDamageType::StaticClass());
			}
			bShouldTriggerScreenShake = true;
		}
	}

	if (bShouldTriggerScreenShake) {
		TriggerScreenShake();
	}

	LightningChargedCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APlayerCharacter::ResetLightningAttack()
{
	LightningStrikeCount = 0;
}

void APlayerCharacter::SpawnOrAdjustDamageCollider(int32 NumLightnings)
{
	switch (NumLightnings) {
	default:
		return;
	case 1:
		LightningChargedCollider->SetBoxExtent(FVector(32.0f, 500.0f, 32.0f));
		LightningChargedCollider->SetRelativeLocation(FVector(0.0f, 510.0f, 120.0f));
		return;
	case 3:
		LightningChargedCollider->SetBoxExtent(FVector(300.0f, 500.0f, 32.0f));
		LightningChargedCollider->SetRelativeLocation(FVector(0.0f, 510.0f, 120.0f));
		return;
	case 5:
		LightningChargedCollider->SetBoxExtent(FVector(500.0f, 500.0f, 32.0f));
		LightningChargedCollider->SetRelativeLocation(FVector(0.0f, 510.0f, 120.0f));
		return;
	}
}

float APlayerCharacter::CalculateColliderSizeBasedOnLightning(int32 NumLightnings)
{
	float BaseSize = 100.0f; 
	float SizeIncrement = 50.0f;
	return BaseSize + (SizeIncrement * (NumLightnings - 1));
}

void APlayerCharacter::OnWeaponColliderOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->ActorHasTag(FName("Enemy")))
	{
		if (!HitActors.Contains(OtherActor))
		{
			HitActors.Add(OtherActor);
			if (bCantGethit) {
				if (bVitalActivated) {
					if (IsAbilityUnlocked("VitalUpgrade")) {
						UGameplayStatics::ApplyDamage(OtherActor, MeleeChargedDamage + MeleeChargedDamage / 2, GetInstigator()->GetController(), this, UDamageType::StaticClass());
					}
					else {
						UGameplayStatics::ApplyDamage(OtherActor, MeleeChargedDamage + MeleeChargedDamage * 0.35, GetInstigator()->GetController(), this, UDamageType::StaticClass());
					}
				}
				else {
					UGameplayStatics::ApplyDamage(OtherActor, MeleeChargedDamage, GetInstigator()->GetController(), this, UDamageType::StaticClass());
				}
			}
			else {
				if (bVitalActivated) {
					if (IsAbilityUnlocked("VitalUpgrade")) {
						UGameplayStatics::ApplyDamage(OtherActor, MeleeDamage + MeleeDamage / 2, GetInstigator()->GetController(), this, UDamageType::StaticClass());
					}
					else {
						UGameplayStatics::ApplyDamage(OtherActor, MeleeDamage + MeleeDamage * 0.35, GetInstigator()->GetController(), this, UDamageType::StaticClass());
					}
				}
				else {
					UGameplayStatics::ApplyDamage(OtherActor, MeleeDamage, GetInstigator()->GetController(), this, UDamageType::StaticClass());
				}
			}

			if (bVitalActivated && IsAbilityUnlocked(TEXT("VitalRoboVida"))) {
				Attributes->HealPotion(2);
				SetHUDHealth();
			}

			bShouldTriggerScreenShake = true;
		}
	}

	if (bShouldTriggerScreenShake) {
		TriggerScreenShake();
	}
}

void APlayerCharacter::OnWeaponColliderActivated()
{
	WeaponCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void APlayerCharacter::OnWeaponColliderDeactivated()
{
	HitActors.Empty();
	WeaponCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APlayerCharacter::OnLightningColliderActivated()
{
	LightningChargedCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TArray<AActor*> OverlappingActors;
	LightningChargedCollider->GetOverlappingActors(OverlappingActors);

	for (AActor* Actor : OverlappingActors)
	{
		if (Actor && Actor != this)
		{
			if (IsAbilityUnlocked(TEXT("RayoDamage"))) {
				UGameplayStatics::ApplyDamage(Actor, LightningPoweredDamage, GetInstigator()->GetController(), this, UDamageType::StaticClass());
			}
			else {
				UGameplayStatics::ApplyDamage(Actor, LightningDamage, GetInstigator()->GetController(), this, UDamageType::StaticClass());
			}
		}
	}
}

void APlayerCharacter::OnLightningColliderDeactivated()
{
	LightningChargedCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APlayerCharacter::AdjustWeaponBox(const FVector& NewSize)
{
	WeaponCollider->SetBoxExtent(NewSize);
}

void APlayerCharacter::AdjustLightningBox(const FVector& NewSize)
{
	LightningChargedCollider->SetBoxExtent(NewSize);
}

void APlayerCharacter::SpawnLightningStrikeFromAnimation()
{
	if (GetWorld())
	{
		FVector StartLocation = GetMesh()->GetSocketLocation(FName("RightHandSocket"));
		FVector ForwardVector = GetActorForwardVector();
		ForwardVector.Normalize(); 

		float TraceDistance = 400.0f;
		FVector EndLocation = StartLocation + ForwardVector * TraceDistance;

		FHitResult HitResult;
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(this);

		bool bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			StartLocation,
			EndLocation,
			ECC_GameTraceChannel3,
			CollisionParams
		);

		FVector SpawnLocation = StartLocation;
		float SpawnDistance = 200.0f;

		if (CombatTarget == nullptr && AimTarget == nullptr) {
			while (!bHit && (SpawnLocation - StartLocation).Size() < TraceDistance)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = this;
				SpawnLightningTep(SpawnLocation);

				SpawnLocation += ForwardVector * SpawnDistance;
			}
			BeamNiagara(StartLocation, EndLocation);
		}
		else {
			AActor* Target = AimTarget ? AimTarget : CombatTarget;
			FVector TargetLocation = Target->GetActorLocation();
			FVector CombatVector = (TargetLocation - StartLocation).GetSafeNormal() * SpawnDistance;
			float DistanceToTarget = (TargetLocation - StartLocation).Size();

			while ((SpawnLocation - StartLocation).Size() < DistanceToTarget)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = this;
				SpawnLightningTep(SpawnLocation);

				SpawnLocation += CombatVector;
				if ((SpawnLocation - StartLocation).Size() >= DistanceToTarget)
					break; 
			}

			FVector EndPoint = bHit ? HitResult.Location : TargetLocation;
			LanzamientoRayo(StartLocation, EndPoint, Target);
		}
	}
}

void APlayerCharacter::LanzamientoRayo(FVector& StartPoint, FVector& EndPoint, AActor* Target)
{
	EndPoint = Target->GetActorLocation();
	if (Target->ActorHasTag("Boss")) {
		FString functionName = TEXT("TryBlock");
		Target->CallFunctionByNameWithArguments(*functionName, *GLog, nullptr, true);
	}
	if (IsAbilityUnlocked(TEXT("RayoDamage"))) {
		UGameplayStatics::ApplyDamage(Target, LightningPoweredDamage, GetInstigator()->GetController(), this, UDamageType::StaticClass());
	}
	else {
		UGameplayStatics::ApplyDamage(Target, LightningDamage, GetInstigator()->GetController(), this, UDamageType::StaticClass());
	}
	CameraShake();
	IHitInterface* HitInterface = Cast<IHitInterface>(Target);
	if (HitInterface)
	{
		HitInterface->Execute_GetHit(Target, Target->GetActorLocation(), GetOwner());
	}
	BeamNiagara(StartPoint, EndPoint);
	if (Target->ActorHasTag("Bateria")) {
		FString functionName = TEXT("TriggerLightningEffect");
		Target->CallFunctionByNameWithArguments(*functionName, *GLog, nullptr, true);
	} else {
		if (IsAbilityUnlocked(TEXT("RayoRebote"))) {
			AActor* NextTarget = FindClosestEnemyAround(Target);
			if (NextTarget)
			{
				if (NextTarget->ActorHasTag("Boss")) {
					FString functionName = TEXT("TryBlock");
					Target->CallFunctionByNameWithArguments(*functionName, *GLog, nullptr, true);
				}
				if (IsAbilityUnlocked(TEXT("RayoDamage"))) {
					UGameplayStatics::ApplyDamage(NextTarget, LightningPoweredDamage, GetInstigator()->GetController(), this, UDamageType::StaticClass());
				}
				else {
					UGameplayStatics::ApplyDamage(NextTarget, LightningDamage, GetInstigator()->GetController(), this, UDamageType::StaticClass());
				}
				
				FVector NextTargetLocation = NextTarget->GetActorLocation();
				BeamNiagara(Target->GetActorLocation(), NextTargetLocation);
			}
		}
	}
}

AActor* APlayerCharacter::FindClosestEnemyAround(AActor* Target)
{
	TArray<AActor*> OverlappedActors;
	float SearchRadius = 300.0f;
	FVector TargetLocation = Target->GetActorLocation();

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	ActorsToIgnore.Add(Target);

	UKismetSystemLibrary::SphereOverlapActors(this, TargetLocation, SearchRadius, ObjectTypes, nullptr, ActorsToIgnore, OverlappedActors);

	AActor* ClosestEnemy = nullptr;
	float MinDistance = FLT_MAX;

	for (AActor* Actor : OverlappedActors)
	{
		if (Actor->Tags.Contains(FName("Enemy")))
		{
			float Distance = FVector::Distance(TargetLocation, Actor->GetActorLocation());
			if (Distance < MinDistance)
			{
				MinDistance = Distance;
				ClosestEnemy = Actor;
			}
		}
	}
	return ClosestEnemy;
}

void APlayerCharacter::HandleHealing()
{
	if ((Attributes->GetMana() > 5)) {
		if (bCargaVida) {
			Attributes->HealPotion(5);
			SetHUDHealth();
			Attributes->UseMana(5);
			PlayerOverlay->SetManaBarPercent(Attributes->GetManaPercent());
			PlayerOverlay->SetManaEnergyBarPercent(Attributes->GetManaPercent());
		}
	}
	else {
		AttackEnd();
		StopAnimMontage();
		bCargaVida = false;
		bcanRotate = true;
		bCanAttack = true;
		bCantGethit = false;
		RemoveChargedVital();
		if (ChargeAudioComponent)
		{
			ChargeAudioComponent->Stop();
		}
		if (TimerHandle_Healing.IsValid()) {
			GetWorldTimerManager().ClearTimer(TimerHandle_Healing);
		}
	}
	
}
void APlayerCharacter::HandleManaVital(){
	if (!(Attributes->GetMana() > 5)) {
		DeactivateVitalAttackEffects();
	}
	else {
		Attributes->UseMana(5);
	}
	PlayerOverlay->SetManaBarPercent(Attributes->GetManaPercent());
	PlayerOverlay->SetManaEnergyBarPercent(Attributes->GetManaPercent());
}

void APlayerCharacter::TriggerScreenShake()
{
	bShouldTriggerScreenShake = false;
	CameraShake();
}

void APlayerCharacter::ConfigureChargeAttackTimeline()
{
	if (ChargeAttackCurve)
	{
		FOnTimelineFloat ProgressFunction;
		ProgressFunction.BindUFunction(this, FName("HandleTimelineProgress"));
		ChargeAttackTimeline->AddInterpFloat(ChargeAttackCurve, ProgressFunction);
	}
}

void APlayerCharacter::HandleTimelineProgress(float Value)
{
	FVector NewLocation = FMath::Lerp(ChargeAttackInitialPosition, ChargeAttackTargetPosition, Value);
	SetActorLocation(NewLocation, true);
}