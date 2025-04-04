// Copyright Epic Games, Inc. All Rights Reserved.

#include "SeSAC_NetworkCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Kismet/GameplayStatics.h"
#include "CNetPlayerAnimInstance.h"
#include "Components/WidgetComponent.h"
#include "CMainUI.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ASeSAC_NetworkCharacter

ASeSAC_NetworkCharacter::ASeSAC_NetworkCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // Character moves in the direction of input...	
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 150.0f; // The camera follows at this distance behind the character	
	CameraBoom->SetRelativeLocation(FVector(0, 40, 60));
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	GunComp = CreateDefaultSubobject<USceneComponent>(TEXT("GunComp"));
	GunComp->SetupAttachment(GetMesh(), TEXT("GunPosition"));

}

void ASeSAC_NetworkCharacter::BeginPlay()
{
	Super::BeginPlay();

	InitUIWidget();

	// 총 검색
	TArray<AActor*> allactors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), allactors);
	for (const auto& actor : allactors)
	{
		if (actor->GetName().Contains("BP_Pistol"))
			PistolActors.Add(actor);
	}

}

//////////////////////////////////////////////////////////////////////////
// Input

void ASeSAC_NetworkCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ASeSAC_NetworkCharacter::TakePistol(const FInputActionValue& Value)
{
	// 총을 소유하지 않았다면 일정 범위 안에 있는 총을 잡는다.

	// 필요 속성 : 총을 소유하고 있는지, 소유중인 총, 총을 잡을 수 있는 범위

	// 1. 총을 잡고 있지 않다면
	if (bHasPistol == true)	return;

	// 2. 월드에 있는 총을 모두 찾는다.
	for (const auto& pistol : PistolActors)
	{
		// 3. 총의 주인이 있다면 그 총은 검사하지 않는다.
		// 총이 없어질 수 있는 경우가 있다면 NULL체크 필요
		if (pistol->GetOwner() != nullptr) continue;

		// 4. 총과의 거리를 구한다.
		float distance = FVector::Dist(GetActorLocation(), pistol->GetActorLocation());

		// 5. 총이 범위 안에 있다면
		if (distance <= DistanceToGun)
		{
			// 6. 소유중인 총으로 등록한다.
			OwnedPistol = pistol;

			// 7. 총의 소유자를 자신으로 등록한다.
			pistol->SetOwner(this);

			// 8. 총 소유 상태를 변경한다.
			bHasPistol = true;

			// 9. 해당 총을 붙인다.
			AttachPistol(pistol);

			break;
		}
	} // for pistol

}

void ASeSAC_NetworkCharacter::ReleasePistol(const FInputActionValue& Value)
{
	// 총을 잡지 않았을 때는 처리하지 않는다.
	if (bHasPistol == false) return;

	// 총 소유시
	if (OwnedPistol)
	{
		DetachPistol(OwnedPistol);

		// 미소유로 설정
		bHasPistol = false;

		OwnedPistol->SetOwner(nullptr);
		OwnedPistol = nullptr;
	}

}

void ASeSAC_NetworkCharacter::Fire(const FInputActionValue& Value)
{
	// 총을 들고 있지 않을때는 처리하지 않는다.
	if (!bHasPistol) return;

	// 총쏘기 애니메이션 재생
	auto anim = Cast<UCNetPlayerAnimInstance>(GetMesh()->GetAnimInstance());
	anim->PlayFireAnimation();

	// 총쏘기
	FHitResult hitInfo;

	FVector startPos = FollowCamera->GetComponentLocation();
	FVector endPos = startPos + FollowCamera->GetForwardVector() * 10000;

	FCollisionQueryParams params;
	params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(hitInfo, startPos, endPos, ECC_Visibility, params);

	if (bHit)
	{
		// 맞는 부위에 파티클 표시
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), GunEffect, hitInfo.Location);
	}

}

void ASeSAC_NetworkCharacter::DetachPistol(AActor* InPistol)
{
	auto mesh = InPistol->GetComponentByClass<UStaticMeshComponent>();

	mesh->SetSimulatePhysics(true);
	mesh->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);

	MainUI->ShowCrossHair(false);

}

void ASeSAC_NetworkCharacter::AttachPistol(AActor* InPistol)
{
	auto mesh = InPistol->GetComponentByClass<UStaticMeshComponent>();
	mesh->SetSimulatePhysics(false);
	mesh->AttachToComponent(GunComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	MainUI->ShowCrossHair(true);

}

void ASeSAC_NetworkCharacter::InitUIWidget()
{
	if (MainUIWidget)
	{
		MainUI = Cast<UCMainUI>(CreateWidget(GetWorld(), MainUIWidget));
		MainUI->AddToViewport();
		MainUI->ShowCrossHair(false);
	}

}

void ASeSAC_NetworkCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASeSAC_NetworkCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASeSAC_NetworkCharacter::Look);

		// TakePistol
		EnhancedInputComponent->BindAction(TakePistolAction, ETriggerEvent::Started, this, &ASeSAC_NetworkCharacter::TakePistol);

		// ReleasePistol
		EnhancedInputComponent->BindAction(ReleasePistolAction, ETriggerEvent::Started, this, &ASeSAC_NetworkCharacter::ReleasePistol);

		// Fire
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ASeSAC_NetworkCharacter::Fire);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ASeSAC_NetworkCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ASeSAC_NetworkCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}
