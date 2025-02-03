#include "Characters/SlashCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Items/Item.h"
#include "Items/Weapons/Weapon.h"
#include "Animation/AnimMontage.h"
#include "Components/BoxComponent.h"

ASlashCharacter::ASlashCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->TargetArmLength = 300.f;

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(SpringArm);
}

void ASlashCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASlashCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASlashCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis(FName("MoveForward"), this, &ASlashCharacter::MoveForward);
	PlayerInputComponent->BindAxis(FName("Turn"), this, &ASlashCharacter::Turn);
	PlayerInputComponent->BindAxis(FName("LookUp"), this, &ASlashCharacter::LookUp);
	PlayerInputComponent->BindAxis(FName("MoveRight"), this, &ASlashCharacter::MoveRight);

	PlayerInputComponent->BindAction(FName("Jump"), IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction(FName("Equip"), IE_Pressed, this, &ASlashCharacter::EKeyPressed);
	PlayerInputComponent->BindAction(FName("Attack"), IE_Pressed, this, &ASlashCharacter::Attack);
}

void ASlashCharacter::SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
	if (EquipWeapon && EquipWeapon->GetWeaponBox())
	{
		EquipWeapon->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);
		EquipWeapon->IgnoreActors.Empty();
	}
}

void ASlashCharacter::MoveForward(float value)
{
	if (ActionState != EActionState::EAS_Unoccupied) return;
	if (Controller && (value != 0))
	{
		const FRotator ControlRotation = GetControlRotation();
		const FRotator YawInput(0.f, ControlRotation.Yaw, 0.f);
		const FVector Direction = FRotationMatrix(YawInput).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, value);
	}
}

void ASlashCharacter::MoveRight(float value)
{
	if (ActionState != EActionState::EAS_Unoccupied) return;
	if (Controller && (value != 0))
	{
		const FRotator ControlRotation = GetControlRotation();
		const FRotator YawInput(0.f, ControlRotation.Yaw, 0.f);
		const FVector Direction = FRotationMatrix(YawInput).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, value);
	}
}

void ASlashCharacter::Turn(float value)
{
	AddControllerYawInput(value);
}

void ASlashCharacter::LookUp(float value)
{
	AddControllerPitchInput(value);
}

void ASlashCharacter::EKeyPressed()
{
	AWeapon* OverlappingWeapon = Cast<AWeapon>(OverlappingItem);
	if (OverlappingWeapon)
	{
		OverlappingWeapon->Equip(GetMesh(), FName("RightHandSocket"));
		CharacterState = ECharacterState::ECS_EquipedOneHandedWeapon;
		OverlappingItem = nullptr;
		EquipWeapon = OverlappingWeapon;
	}
	else
	{
		if (CanDisArm())
		{
			PlayEquippedMontage(FName("UnequipWeapon"));
			CharacterState = ECharacterState::ECS_Unequiped;
			ActionState = EActionState::EAS_EquippingWeapon;
		}
		else if (CanArm())
		{
			PlayEquippedMontage(FName("EquipWeapon"));
			CharacterState = ECharacterState::ECS_EquipedOneHandedWeapon;
			ActionState = EActionState::EAS_EquippingWeapon;
		}
	}
}

void ASlashCharacter::Attack()
{
	if (CanAttack())
	{
		PlayAttackMontage();
		ActionState = EActionState::EAS_Attacking;
	}
}

bool ASlashCharacter::CanAttack()
{
	return ActionState == EActionState::EAS_Unoccupied && CharacterState !=ECharacterState::ECS_Unequiped;
}

bool ASlashCharacter::CanDisArm()
{
	return ActionState==EActionState::EAS_Unoccupied &&
		CharacterState!=ECharacterState::ECS_Unequiped;
}

bool ASlashCharacter::CanArm()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState == ECharacterState::ECS_Unequiped &&
		EquipWeapon;
}

void ASlashCharacter::DisArm()
{
	if (EquipWeapon)
	{
		EquipWeapon->AttachMeshToSocket(GetMesh(), FName("SpineSocket"));
	}
}

void ASlashCharacter::Arm()
{
	if (EquipWeapon)
	{
		EquipWeapon->AttachMeshToSocket(GetMesh(), FName("RightHandSocket"));
	}
}

void ASlashCharacter::FinishEquipping()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void ASlashCharacter::PlayAttackMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage)
	{
		AnimInstance->Montage_Play(AttackMontage);
		const int32 Selection = FMath::RandRange(0, 1);
		FName SectionName = FName();
		switch (Selection)
		{
		case 0:
			SectionName = FName("Attack1");
			break;
		case 1:
			SectionName = FName("Attack2");
			break;
		default:
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName, AttackMontage);
	}
}

void ASlashCharacter::PlayEquippedMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EquippedMontage)
	{
		AnimInstance->Montage_Play(EquippedMontage);
		AnimInstance->Montage_JumpToSection(SectionName, EquippedMontage);
	}
}

void ASlashCharacter::AttackEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

