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
#include "CHealthBar.h"
#include "SeSAC_Network.h"
#include "Net/UnrealNetwork.h"
#include "Components/HorizontalBox.h"
#include "CNetPlayerController.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ASeSAC_NetworkCharacter

ASeSAC_NetworkCharacter::ASeSAC_NetworkCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

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

	HpUIComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("HpUIComp"));
	HpUIComp->SetupAttachment(GetMesh());

}

void ASeSAC_NetworkCharacter::BeginPlay()
{
	Super::BeginPlay();

	// InitUIWidget();

	// 클라이언트
	if (IsLocallyControlled() and HasAuthority() == false)
	{
		// UI 위젯 초기화
		InitUIWidget();
	}

	// 총 검색
	TArray<AActor*> allactors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), allactors);
	for (const auto& actor : allactors)
	{
		if (actor->GetName().Contains("BP_Pistol"))
			PistolActors.Add(actor);
	}

}

void ASeSAC_NetworkCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// HP Bar 빌보드
	if (HpUIComp and HpUIComp->GetVisibleFlag())
	{
		FVector camLocation = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetCameraLocation();
		FVector direction = camLocation - HpUIComp->GetComponentLocation();
		direction.Z = 0;

		HpUIComp->SetWorldRotation(direction.GetSafeNormal().ToOrientationRotator());
	}

	PrintNetLog();

}

void ASeSAC_NetworkCharacter::PrintNetLog()
{
	const FString validConnection = GetNetConnection() != nullptr ? TEXT("Valid Connection") : TEXT("Invalid Connection");

	const FString ownerName = GetOwner() != nullptr ? GetOwner()->GetName() : TEXT("No Owner");

	const FString logStr = FString::Printf(TEXT("Connection : %s\n Owner Name : %s\n Local Role : %s\n Remote Role : %s"), *validConnection, *ownerName, *LOCAL_ROLE, *REMOTE_ROLE);

	DrawDebugString(GetWorld(), GetActorLocation() + GetActorUpVector() * 100, *logStr, nullptr, FColor::White, 0, true, 1);

	// 권한(Authority)
	// ROLE_Authority : 모든 권한을 다 가지고 있음 (로직 실행 가능)
	// HasAuthority()
	// 
	// ROLE_AutonomouseProxy : 제어(Input)만 가능
	// IsLocallyControlled() : 제어권을 갖고 있는지
	// 
	// ROLE_SimulatedProxy : 시뮬레이션만 가능

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

	ServerPRC_TakePistol(); // 클라이언트에서 서버로 요청 (클라이언트)

}

void ASeSAC_NetworkCharacter::ReleasePistol(const FInputActionValue& Value)
{
	// 총을 잡고 있지 않거나 재장전 중이거나, 로컬 사용자가 아니라면 처리하지 않는다.
	if (bHasPistol == false or IsReloading or !IsLocallyControlled()) return;

	ServerRPC_ReleasePistol(); // 클라이언트에서 서버로 요청 (클라이언트)

}

void ASeSAC_NetworkCharacter::Fire(const FInputActionValue& Value)
{
	// 총을 들고 있지 않을때 또는 총알이 없을 경우 처리하지 않는다.
	if (!bHasPistol or IsReloading or BulletCount <= 0) return;

	ServerRPC_Fire();

}

void ASeSAC_NetworkCharacter::DetachPistol(AActor* InPistol)
{
	auto mesh = InPistol->GetComponentByClass<UStaticMeshComponent>();

	mesh->SetSimulatePhysics(true);
	mesh->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);

	//MainUI->ShowCrossHair(false);

	if (IsLocallyControlled() and MainUI)
		MainUI->ShowCrossHair(false);

}

void ASeSAC_NetworkCharacter::AttachPistol(AActor* InPistol)
{
	auto mesh = InPistol->GetComponentByClass<UStaticMeshComponent>();
	mesh->SetSimulatePhysics(false);
	mesh->AttachToComponent(GunComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	//MainUI->ShowCrossHair(true);

	if (IsLocallyControlled() and MainUI)
		MainUI->ShowCrossHair(true);

}

void ASeSAC_NetworkCharacter::InitUIWidget()
{
	PRINTLOG(TEXT("[%s] Begin"), Controller ? TEXT("Player") : TEXT("Not Player"));

	// Player가 제어중이 아니라면 처리하지 않는다.
	auto pc = Cast<ACNetPlayerController>(Controller);
	if (!pc) return;

	if (pc->MainUIWidget)
	{
		if (pc->MainUI == nullptr)
			pc->MainUI = Cast<UCMainUI>(CreateWidget(GetWorld(), pc->MainUIWidget));

		MainUI = pc->MainUI;
		MainUI->AddToViewport();
		MainUI->ShowCrossHair(false);

		hp = MaxHP;
		MainUI->HP = 1;

		// 총알 모두 제거
		MainUI->RemoveAllAmmo();
		BulletCount = MaxBulletCount;

		// 총알 추가
		for (int i = 0; i < MaxBulletCount; ++i)
			MainUI->AddBullet();

		if (HpUIComp) // HP UI 숨김 처리
			HpUIComp->SetVisibility(false);
	}

}

void ASeSAC_NetworkCharacter::ReloadPistol(const FInputActionValue& Value)
{
	// 총 소지중이 아니거나 재장전 중이라면 아무 처리하지 않는다.
	if (!bHasPistol or BulletCount == MaxBulletCount or IsReloading) return;

	// 재장전 애니메이션 재생
	auto anim = Cast<UCNetPlayerAnimInstance>(GetMesh()->GetAnimInstance());
	anim->PlayReloadAnimation();

	IsReloading = true;
}

void ASeSAC_NetworkCharacter::InitAmmoUI()
{
	ServerRPC_Reload();

}

void ASeSAC_NetworkCharacter::OnRep_HP()
{
	// 사망처리
	if (HP <= 0)
	{
		IsDead = true;

		ReleasePistol(FInputActionValue());

		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		GetCharacterMovement()->DisableMovement();
	}

	// UI에 할당할 퍼센트 계산
	float percent = hp / MaxHP;

	if (MainUI)
	{
		MainUI->HP = percent;

		// 피격 효과 처리
		MainUI->PlayDamageAnimation();

		// 카메라 쉐이크
		if (DamageCameraShake)
		{
			auto pc = Cast<APlayerController>(Controller);
			pc->ClientStartCameraShake(DamageCameraShake);
		}
	}
	else
	{
		auto hpUI = Cast<UCHealthBar>(HpUIComp->GetWidget());
		hpUI->HP = percent;
	}

}

float ASeSAC_NetworkCharacter::GetHP()
{
	return hp;
}

void ASeSAC_NetworkCharacter::SetHP(float value)
{
	hp = value;

	OnRep_HP();

}

void ASeSAC_NetworkCharacter::DamageProcess()
{
	// 체력을 감소시킨다.
	HP--;

	//if (HP <= 0)
	//	IsDead = true;

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

		// Reload
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &ASeSAC_NetworkCharacter::ReloadPistol);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

// 클라이언트에게 요청받은 내용을 처리 (서버)
void ASeSAC_NetworkCharacter::ServerPRC_TakePistol_Implementation()
{
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

			MulticastRPC_TakePistol(pistol); // 요청 받은 내용을 검증 후 클라이언트에게 명령 (서버)

			break;
		}
	} // for pistol

}

// 서버에서 클라이언트에게 명령한 내용을 클라이언트에서 실행 (클라이언트)
void ASeSAC_NetworkCharacter::MulticastRPC_TakePistol_Implementation(AActor* InPistolActor)
{
	// 9. 해당 총을 붙인다.
	AttachPistol(InPistolActor);

}

void ASeSAC_NetworkCharacter::ServerRPC_ReleasePistol_Implementation()
{
	// 총 소유시
	if (OwnedPistol)
	{
		MulticastRPC_ReleasePistol(OwnedPistol);

		// 미소유로 설정
		bHasPistol = false;

		OwnedPistol->SetOwner(nullptr);
		OwnedPistol = nullptr;
	}

}

void ASeSAC_NetworkCharacter::MulticastRPC_ReleasePistol_Implementation(AActor* InPistolActor)
{
	DetachPistol(InPistolActor);

}

void ASeSAC_NetworkCharacter::ServerRPC_Fire_Implementation()
{
	// 총알 제거
	BulletCount--;

	// 총쏘기
	FHitResult hitInfo;

	FVector startPos = FollowCamera->GetComponentLocation();
	FVector endPos = startPos + FollowCamera->GetForwardVector() * 10000;

	FCollisionQueryParams params;
	params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(hitInfo, startPos, endPos, ECC_Visibility, params);

	if (bHit)
	{
		// 맞은 대상이 상대방일 경우 데미지 처리
		auto other = Cast<ASeSAC_NetworkCharacter>(hitInfo.GetActor());
		if (other)
			other->DamageProcess();
	}

	MulticastRPC_Fire(bHit, hitInfo);

}

void ASeSAC_NetworkCharacter::MulticastRPC_Fire_Implementation(bool InbHit, const FHitResult& InHitInfo)
{
	if (InbHit)
	{
		// 맞는 부위에 파티클 표시
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), GunEffect, InHitInfo.Location);
	}

	if (MainUI)
	{
		// 총알 제거
		MainUI->PopBullet(BulletCount);
	}

	// 총쏘기 애니메이션 재생
	auto anim = Cast<UCNetPlayerAnimInstance>(GetMesh()->GetAnimInstance());
	anim->PlayFireAnimation();

}

void ASeSAC_NetworkCharacter::ServerRPC_Reload_Implementation()
{
	// 총알 개수 초기화
	BulletCount = MaxBulletCount;

	ClientRPC_Reload();

}

void ASeSAC_NetworkCharacter::ClientRPC_Reload_Implementation()
{
	if (MainUI)
	{
		// 총알 UI 제거
		MainUI->RemoveAllAmmo();

		// 총알 UI를 다시 셋팅
		for (int i = 0; i < MaxBulletCount; ++i)
			MainUI->AddBullet();
	}

	// 재장전 완료 상태로 처리
	IsReloading = false;

}

void ASeSAC_NetworkCharacter::DieProcess()
{
	auto pc = Cast<APlayerController>(Controller);
	pc->SetShowMouseCursor(true);
	GetFollowCamera()->PostProcessSettings.ColorSaturation = FVector4(0, 0, 0, 1);

	// Die UI 표시
	MainUI->GameOverUI->SetVisibility(ESlateVisibility::Visible);

}

void ASeSAC_NetworkCharacter::PossessedBy(AController* NewController)
{
	PRINTLOG(TEXT("Begin"));
	Super::PossessedBy(NewController);

	if (IsLocallyControlled())
		InitUIWidget();

	PRINTLOG(TEXT("End"));

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

void ASeSAC_NetworkCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASeSAC_NetworkCharacter, bHasPistol);

	DOREPLIFETIME(ASeSAC_NetworkCharacter, BulletCount);

	DOREPLIFETIME(ASeSAC_NetworkCharacter, hp);

}
