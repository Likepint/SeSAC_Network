#include "CMainUI.h"
#include "Components/Image.h"
#include "Components/UniformGridPanel.h"
#include "Animation/WidgetAnimation.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "CNetPlayerController.h"

void UCMainUI::ShowCrossHair(bool bShow)
{
	if (bShow)
		 IMG_CrossHair->SetVisibility(ESlateVisibility::Visible);
	else IMG_CrossHair->SetVisibility(ESlateVisibility::Hidden);

}

void UCMainUI::AddBullet()
{
	auto bullet = CreateWidget(GetWorld(), BulletUIFactory);
	BulletPanel->AddChildToUniformGrid(bullet, 1, BulletPanel->GetChildrenCount());

}

void UCMainUI::PopBullet(int32 index)
{
	BulletPanel->RemoveChildAt(index);

}

void UCMainUI::RemoveAllAmmo()
{
	for (auto& bullet : BulletPanel->GetAllChildren())
		BulletPanel->RemoveChild(bullet);

}

void UCMainUI::PlayDamageAnimation()
{
	PlayAnimation(DamageAnimation);

}

void UCMainUI::NativeConstruct()
{
	Super::NativeConstruct();

	Button_Retry->OnClicked.AddDynamic(this, &UCMainUI::OnRetry);
	Button_Exit->OnClicked.AddDynamic(this, &UCMainUI::OnExit);
}

void UCMainUI::OnRetry()
{
	// 게임종료 UI 안보이도록 처리
	GameOverUI->SetVisibility(ESlateVisibility::Hidden);

	if (auto pc = Cast<ACNetPlayerController>(GetOwningPlayer()))
	{
		// 마우스 커서 안보이도록 처리
		pc->SetShowMouseCursor(false);

		// 관전자로 변경 요청
		//pc->ServerRPC_RespawnPlayer();
		pc->ServerRPC_ChangeToSpectator();
	}

}

void UCMainUI::OnExit()
{

}
