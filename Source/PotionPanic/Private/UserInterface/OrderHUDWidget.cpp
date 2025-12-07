#include "UserInterface/OrderHUDWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/Texture2D.h"

void UOrderHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();

    EnsureFallbackWidgets();
    SetVisibility(ESlateVisibility::Hidden);
}

void UOrderHUDWidget::UpdateOrder(const FClientOrderEntry& Order, float RemainingTime, bool bIsActive)
{
    EnsureFallbackWidgets();

    if (!bIsActive)
    {
        ClearStatus();
        SetVisibility(ESlateVisibility::Hidden);
        return;
    }

    SetVisibility(ESlateVisibility::HitTestInvisible);

    UpdateOrderName(Order);
    UpdateIcon(Order);
    UpdateTimer(RemainingTime);
    ClearStatus();
}

void UOrderHUDWidget::ShowResult(bool bSuccess)
{
    EnsureFallbackWidgets();

    SetVisibility(ESlateVisibility::HitTestInvisible);

    if (Text_Status)
    {
        Text_Status->SetText(bSuccess ? SuccessText : FailureText);
    }
}

void UOrderHUDWidget::EnsureFallbackWidgets()
{
    if (Image_OrderIcon && Text_OrderName && Text_Timer && Text_Status)
    {
        return;
    }

    if (!WidgetTree)
    {
        return;
    }

    if (!WidgetTree->RootWidget)
    {
        UVerticalBox* RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Root"));
        WidgetTree->RootWidget = RootBox;
    }

    UPanelWidget* RootPanel = Cast<UPanelWidget>(WidgetTree->RootWidget);
    if (!RootPanel)
    {
        return;
    }

    if (!Image_OrderIcon)
    {
        Image_OrderIcon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("OrderIcon"));
        if (Image_OrderIcon)
        {
            Image_OrderIcon->SetDesiredSizeOverride(FVector2D(128.f, 128.f));
            RootPanel->AddChild(Image_OrderIcon);
        }
    }

    if (!Text_OrderName)
    {
        Text_OrderName = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("OrderName"));
        if (Text_OrderName)
        {
            Text_OrderName->SetJustification(ETextJustify::Center);
            RootPanel->AddChild(Text_OrderName);
        }
    }

    if (!Text_Timer)
    {
        Text_Timer = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("OrderTimer"));
        if (Text_Timer)
        {
            Text_Timer->SetJustification(ETextJustify::Center);
            RootPanel->AddChild(Text_Timer);
        }
    }

    if (!Text_Status)
    {
        Text_Status = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("OrderStatus"));
        if (Text_Status)
        {
            Text_Status->SetJustification(ETextJustify::Center);
            RootPanel->AddChild(Text_Status);
        }
    }
}

void UOrderHUDWidget::UpdateOrderName(const FClientOrderEntry& Order)
{
    if (!Text_OrderName)
    {
        return;
    }

    Text_OrderName->SetText(ResolveOrderName(Order));
}

void UOrderHUDWidget::UpdateIcon(const FClientOrderEntry& Order)
{
    if (!Image_OrderIcon)
    {
        return;
    }

    if (UTexture2D* Icon = ResolveIcon(Order))
    {
        Image_OrderIcon->SetVisibility(ESlateVisibility::Visible);
        Image_OrderIcon->SetBrushFromTexture(Icon);
    }
    else
    {
        Image_OrderIcon->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UOrderHUDWidget::UpdateTimer(float RemainingTime)
{
    if (!Text_Timer)
    {
        return;
    }

    const int32 TimeInt = FMath::Max(0, FMath::CeilToInt(RemainingTime));
    Text_Timer->SetText(FText::Format(
        NSLOCTEXT("OrderUI", "TimerFormat", "Temps : {0}s"),
        FText::AsNumber(TimeInt)
    ));
}

void UOrderHUDWidget::ClearStatus()
{
    if (Text_Status)
    {
        Text_Status->SetText(FText::GetEmpty());
    }
}

FText UOrderHUDWidget::ResolveOrderName(const FClientOrderEntry& Order) const
{
    if (!Order.DisplayText.IsEmpty())
    {
        return Order.DisplayText;
    }

    if (Order.OrderId != NAME_None)
    {
        return FText::FromName(Order.OrderId);
    }

    return UnknownOrderText;
}

UTexture2D* UOrderHUDWidget::ResolveIcon(const FClientOrderEntry& Order)
{
    if (Order.Icon.IsValid())
    {
        return Order.Icon.Get();
    }

    if (!Order.Icon.IsNull())
    {
        return Order.Icon.LoadSynchronous();
    }

    return nullptr;
}
