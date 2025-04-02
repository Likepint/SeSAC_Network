// Copyright Epic Games, Inc. All Rights Reserved.

#include "SeSAC_NetworkGameMode.h"
#include "SeSAC_NetworkCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASeSAC_NetworkGameMode::ASeSAC_NetworkGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
