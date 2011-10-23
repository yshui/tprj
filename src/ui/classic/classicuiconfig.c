#include "fcitx-config/fcitx-config.h"
#include "classicui.h"

static void FilterCopyUseTray(GenericConfig* config, ConfigGroup *group, ConfigOption *option, void *value, ConfigSync sync, void *filterArg);
static void FilterScreenSizeX(GenericConfig* config, ConfigGroup *group, ConfigOption *option, void *value, ConfigSync sync, void *filterArg);
static void FilterScreenSizeY(GenericConfig* config, ConfigGroup *group, ConfigOption *option, void *value, ConfigSync sync, void *filterArg);

CONFIG_BINDING_BEGIN(FcitxClassicUI)
CONFIG_BINDING_REGISTER_WITH_FILTER("ClassicUI", "MainWindowOffsetX", iMainWindowOffsetX, FilterScreenSizeX)
CONFIG_BINDING_REGISTER_WITH_FILTER("ClassicUI", "MainWindowOffsetY", iMainWindowOffsetY, FilterScreenSizeY)
CONFIG_BINDING_REGISTER("ClassicUI", "Font", font)
CONFIG_BINDING_REGISTER("ClassicUI", "MenuFont", menuFont)
#ifndef _ENABLE_PANGO
CONFIG_BINDING_REGISTER("ClassicUI", "FontLocale", strUserLocale)
#endif
CONFIG_BINDING_REGISTER_WITH_FILTER("ClassicUI", "UseTray", bUseTrayIcon_, FilterCopyUseTray)
CONFIG_BINDING_REGISTER("ClassicUI", "SkinType", skinType)
CONFIG_BINDING_REGISTER("ClassicUI", "MainWindowHideMode", hideMainWindow)
CONFIG_BINDING_REGISTER("ClassicUI", "VerticalList", bVerticalList)
CONFIG_BINDING_END()

void FilterCopyUseTray(GenericConfig* config, ConfigGroup *group, ConfigOption *option, void *value, ConfigSync sync, void *filterArg)
{
    static boolean firstRunOnUseTray = true;
    FcitxClassicUI *classicui = (FcitxClassicUI*) config;
    boolean *b = (boolean*)value;
    if (sync == Raw2Value && b) {
        if (firstRunOnUseTray)
            classicui->bUseTrayIcon = *b;
        firstRunOnUseTray = false;
    }
}

static void FilterScreenSizeX(GenericConfig* config, ConfigGroup *group, ConfigOption *option, void *value, ConfigSync sync, void *filterArg)
{
    int* X = (int*) value;
    FcitxClassicUI *classicui = (FcitxClassicUI*) config;
    int width, height;
    GetScreenSize(classicui, &width, &height);

    switch (sync) {
    case Raw2Value:
        if (*X >= width)
            *X = width - 10;
        if (*X < 0)
            *X = 0;
        break;
    case Value2Raw:
        break;
    }

}

static void FilterScreenSizeY(GenericConfig* config, ConfigGroup *group, ConfigOption *option, void *value, ConfigSync sync, void *filterArg)
{
    int* Y = (int*) value;
    FcitxClassicUI *classicui = (FcitxClassicUI*) config;
    int width, height;
    GetScreenSize(classicui, &width, &height);

    switch (sync) {
    case Raw2Value:
        if (*Y >= height)
            *Y = height - 10;
        if (*Y < 0)
            *Y = 0;
        break;
    case Value2Raw:
        break;
    }


}

// kate: indent-mode cstyle; space-indent on; indent-width 0;
