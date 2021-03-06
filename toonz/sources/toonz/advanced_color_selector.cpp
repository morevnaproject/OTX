#include <advanced_color_selector.hpp>

#include "floatingpanelcommand.h"
#include "pane.h"
#include "tapp.h"
#include <toonz/palettecontroller.h>
#include "tpalette.h"
#include <toonz/tpalettehandle.h>

using color_widgets::AdvancedColorSelector;

TPixelRGBM32 qColorToTPixel(QColor c) {
  return { c.red(), c.green(), c.blue(), c.alpha() };
}

QColor tPixelToQColor(TPixelRGBM32 pix) {
  return { pix.r, pix.g, pix.b, pix.m };
}

class AdvancedColorSelectorFactory final : public TPanelFactory {
public:
  AdvancedColorSelectorFactory() : TPanelFactory("AdvancedColorSelector") {}
  void initialize(TPanel *panel) override {
    auto wheel = new AdvancedColorSelector(panel);
    wheel->setEnabledWidgets(AdvancedColorSelector::RGBSliders);
    auto palette_controller = TApp::instance()->getPaletteController();
    auto palette_handle = palette_controller->getCurrentPalette();
    auto getCurrentColor = [wheel](){
      auto palette_controller = TApp::instance()->getPaletteController();
      auto palette_handle = palette_controller->getCurrentPalette();
      auto palette = palette_handle->getPalette();
      auto styleIndex = palette_handle->getStyleIndex();
      return palette->getStyle(styleIndex)->getColorParamValue(wheel->paramIndex());
    };
    auto setCurrentColor = [wheel](TPixel c) {
      auto palette_controller = TApp::instance()->getPaletteController();
      auto palette_handle = palette_controller->getCurrentPalette();
      auto palette = palette_handle->getPalette();
      auto styleIndex = palette_handle->getStyleIndex();
      palette->getStyle(styleIndex)->setColorParamValue(wheel->paramIndex(), c);
    };
    QObject::connect(
      wheel,
      &AdvancedColorSelector::colorChanged,
      [palette_handle, getCurrentColor, setCurrentColor](QColor c){
        auto palette = palette_handle->getPalette();
        auto styleIndex = palette_handle->getStyleIndex();
        if (!palette || styleIndex < 0)
          return;
	auto tc = qColorToTPixel(c);
	if (getCurrentColor() == tc)
	  return;
	setCurrentColor(tc);
        palette_handle->notifyColorStyleChanged(true);
      }
    );
    auto update_wheel = [palette_handle, wheel, getCurrentColor]() {
      auto palette = palette_handle->getPalette();
      auto styleIndex = palette_handle->getStyleIndex();
      if (!palette || styleIndex < 0)
        return;
      wheel->setColor(tPixelToQColor(getCurrentColor()));
      wheel->saveToHistory();
    };
    QObject::connect(
      palette_handle,
      &TPaletteHandle::colorStyleSwitched,
      update_wheel
    );
    QObject::connect(
      palette_handle,
      &TPaletteHandle::colorStyleChanged,
      update_wheel
    );
    QObject::connect(
      palette_handle,
      &TPaletteHandle::paletteSwitched,
      update_wheel
    );
    
    auto layout = new QVBoxLayout();
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(wheel);
    auto widget = new QWidget();
    widget->setLayout(layout);
    panel->setWidget(widget);

    panel->setIsMaximizable(false);
  }
} advancedColorSelectorFactory;

OpenFloatingPanel openAdvancedColorSelectorCommand(
  "MI_OpenAdvancedColorSelector",
  "AdvancedColorSelector",
  QObject::tr("Advanced Color Selector")
);
