#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CHealthBar.generated.h"

UCLASS()
class SESAC_NETWORK_API UCHealthBar : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "HP")
	float HP = 1;

};
