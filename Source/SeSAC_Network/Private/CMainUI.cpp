#include "CMainUI.h"
#include "Components/Image.h"
#include "Components/UniformGridPanel.h"

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
