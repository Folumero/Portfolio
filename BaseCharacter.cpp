// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/BaseCharacter.h"
#include "Components/BoxComponent.h"
#include "Items/Weapons/Weapon.h"
#include "Components/AttributeComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "BaseProyectil.h"
#include "Projectiles/LightningStrike.h"

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	Attributes = CreateDefaultSubobject<UAttributeComponent>(TEXT("Attributes"));
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();


}

void ABaseCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	if (IsAlive() && Hitter)
	{
		if (!bIsInvincible) {
			DirectionalHitReact(Hitter->GetActorLocation());
			bIsInvincible = true;
			GetWorld()->GetTimerManager().SetTimer(InvincibilityTimerHandle, this, &ABaseCharacter::ResetInvincibility, InvincibilityDuration, false);
		}
	}
	else Die();
	PlayHitSound(ImpactPoint);
}

void ABaseCharacter::ResetInvincibility()
{
	bIsInvincible = false;
}

void ABaseCharacter::ResetDamageImmunity()
{
	bCanReceiveDamage = true;
}

void ABaseCharacter::Attack()
{
	if (CombatTarget && CombatTarget->ActorHasTag(FName("Dead")))
	{
		CombatTarget = nullptr;
	}
}

void ABaseCharacter::Die()
{
	Tags.Add(FName("Dead"));

	PlayDeathMontage();
}

void ABaseCharacter::DirectionalHitReact(const FVector& ImpactPoint)
{
	const FVector Forward = GetActorForwardVector();
	// Lower Impact Point to the Enemy's Actor Location Z
	const FVector ImpactLowered(ImpactPoint.X, ImpactPoint.Y, GetActorLocation().Z);
	const FVector ToHit = (ImpactLowered - GetActorLocation()).GetSafeNormal();

	// Forward * ToHit = |Forward||ToHit| * cos(theta)
	// |Forward| = 1, |ToHit| = 1, so Forward * ToHit = cos(theta)
	const double CosTheta = FVector::DotProduct(Forward, ToHit);
	// Take the inverse cosine (arc-cosine) of cos(theta) to get theta
	double Theta = FMath::Acos(CosTheta);
	// convert from radians to degrees
	Theta = FMath::RadiansToDegrees(Theta);

	// if CrossProduct points down, Theta should be negative
	const FVector CrossProduct = FVector::CrossProduct(Forward, ToHit);
	if (CrossProduct.Z < 0)
	{
		Theta *= -1.f;
	}

	FName Section("FromBack");

	if (Theta >= -45.f && Theta < 45.f)
	{
		Section = FName("FromFront");
	}
	else if (Theta >= -135.f && Theta < -45.f)
	{
		Section = FName("FromLeft");
	}
	else if (Theta >= 45.f && Theta < 135.f)
	{
		Section = FName("FromRight");
	}

	PlayHitReactMontage(Section);
}

void ABaseCharacter::PlayHitSound(const FVector& ImpactPoint)
{
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			HitSound,
			ImpactPoint
		);
	}
}

void ABaseCharacter::SpawnHitParticles()
{
	if (HitParticles)
	{
		UNiagaraComponent* ProjectileBeam = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitParticles, GetActorLocation());
	}
}

void ABaseCharacter::HandleDamage(float DamageAmount)
{
	if (bCanReceiveDamage && Attributes)
	{
		Attributes->ReceiveDamage(DamageAmount);
		bCanReceiveDamage = false;
		GetWorld()->GetTimerManager().SetTimer(DamageImmunityTimerHandle, this, &ABaseCharacter::ResetDamageImmunity, DamageImmunityDuration, false);
	}
}

void ABaseCharacter::PlayMontageSection(UAnimMontage* Montage, const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && Montage)
	{
		AnimInstance->Montage_Play(Montage);
		AnimInstance->Montage_JumpToSection(SectionName, Montage);
	}
}

int32 ABaseCharacter::PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames)
{
	if (SectionNames.Num() <= 0) return -1;
	const int32 MaxSectionIndex = SectionNames.Num() - 1;
	const int32 Selection = FMath::RandRange(0, MaxSectionIndex);
	PlayMontageSection(Montage, SectionNames[Selection]);
	return Selection;
}

int32 ABaseCharacter::PlayAttackMontage()
{
	return PlayRandomMontageSection(MeleeMontage, AttackMontageSections);
}

int32 ABaseCharacter::PlayDeathMontage()
{
	const int32 Selection = PlayRandomMontageSection(DeathMontage, DeathMontageSections);
	TEnumAsByte<EDeathPose> Pose(Selection);
	if (Pose < EDeathPose::EDP_MAX)
	{
		DeathPose = Pose;
	}

	return Selection;
}

void ABaseCharacter::SpawnProjectileFromAnimation(int32 Index)
{
	if (ProjectileTypes.IsValidIndex(Index))
	{
		if (ProjectileTypes[Index])
		{
			FVector SpawnLocation = GetMesh()->GetSocketLocation(FName("RightHandSocket"));
			FRotator SpawnRotation = GetActorRotation();

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;


			ABaseProyectil* SpawnedProjectile = GetWorld()->SpawnActor<ABaseProyectil>(ProjectileTypes[Index], SpawnLocation, SpawnRotation, SpawnParams);
			if (SpawnedProjectile)
			{
				// Configuración adicional del proyectil spawnado si es necesario
			}
		}
	}
}
//FVector StartLocation = GetMesh()->GetSocketLocation(FName("RightHandSocket"));
//FRotator SpawnRotation = GetActorRotation();
//FVector ShootDirection = SpawnRotation.Vector();


void ABaseCharacter::BeamNiagara(const FVector& StartLocation, const FVector& EndPoint)
{
	UNiagaraComponent* ProjectileBeam = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), LightningEffectSystem, StartLocation);

	if (ProjectileBeam)

	{
		ProjectileBeam->SetNiagaraVariableVec3(FString("PosicionFinal"), EndPoint);

	}
}

void ABaseCharacter::SelectAttackMontageSection(UAnimMontage* Montage)
{

	if (PreviousMontage && PreviousMontage != Montage) {
		AttackCount = 0;
	}

	PreviousMontage = Montage;
	AttackCount++;
	FName SectionName = FName("Attack");
	if (AttackCount <= MaxCombo) 
	{
		SectionName.SetNumber(AttackCount + 1);
		PlayMontageSection(Montage, SectionName);
	}
	else 
	{
		ComboEnd();
		SectionName.SetNumber(AttackCount + 1);
		PlayMontageSection(Montage, SectionName);
	}
}

void ABaseCharacter::ComboAdd() 
{
	AttackCount++;
}

void ABaseCharacter::ComboEnd()
{
	AttackCount = 0;
}

void ABaseCharacter::PlayDodgeMontage(const FName& SectionName)
{
	PlayMontageSection(DodgeMontage, SectionName);
}

void ABaseCharacter::StopAttackMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Stop(0.25f, MeleeMontage);
	}
}

FVector ABaseCharacter::GetTranslationWarpTarget()
{
	if(CombatTarget==nullptr) return FVector();
	
	const FVector CombatTargetLocation = CombatTarget->GetActorLocation();
	const FVector Location = GetActorLocation();

	FVector TargetToMe = (Location - CombatTargetLocation).GetSafeNormal();
	TargetToMe *= WarpTargetDistance;

	return CombatTargetLocation + TargetToMe;
}

FVector ABaseCharacter::GetRotationWarpTarget()
{
	if (CombatTarget)
	{
		return CombatTarget->GetActorLocation();
	}
	return FVector();
}

void ABaseCharacter::DisableCapsule()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABaseCharacter::PlayHitReactMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		AnimInstance->Montage_JumpToSection(SectionName, HitReactMontage);
	}
}

bool ABaseCharacter::CanAttack()
{
	return false;
}

bool ABaseCharacter::IsAlive()
{
	return Attributes && Attributes->IsAlive();
}

void ABaseCharacter::DisableMeshCollision()
{
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABaseCharacter::AttackEnd()
{
}

void ABaseCharacter::DodgeEnd()
{
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseCharacter::SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
	if (EquippedWeapon && EquippedWeapon->GetWeaponBox())
	{
		EquippedWeapon->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);
		EquippedWeapon->IgnoreActors.Empty();
	}
}

void ABaseCharacter::ActivateCollider()
{
	if (EquippedWeapon && EquippedWeapon->GetWeaponBox())
	{
		EquippedWeapon->ActivateCollider();
		EquippedWeapon->IgnoreActors.Empty();
	}
}

void ABaseCharacter::AdjustCollisionBox(const FVector& NewSize)
{
	if (EquippedWeapon && EquippedWeapon->GetWeaponBox())
	{
		EquippedWeapon->GetWeaponBox()->SetBoxExtent(NewSize);
	}
}