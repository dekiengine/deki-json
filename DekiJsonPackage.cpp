#include "DekiJsonPackage.h"
#include <deki/interop/Plugin.h>
#include <deki/LogSystem.h>

#ifdef DEKI_EDITOR
extern void DekiJsonRegisterComponents();
extern int DekiJsonGetAutoComponentCount();
extern const Deki::ComponentMeta* DekiJsonGetAutoComponentMeta(int index);
#endif

static bool s_JsonRegistered = false;

extern "C"
{
    DEKI_JSON_API int DekiJsonEnsureRegistered(void)
    {
#ifdef DEKI_EDITOR
        if (s_JsonRegistered)
        {
            return DekiJsonGetAutoComponentCount();
        }
        s_JsonRegistered = true;
        DekiJsonRegisterComponents();
        return DekiJsonGetAutoComponentCount();
#else
        return 0;
#endif
    }

    DEKI_PLUGIN_API const char* DekiPluginGetName(void)
    {
        return "Deki JSON Package";
    }
    DEKI_PLUGIN_API const char* DekiPluginGetVersion(void)
    {
#ifdef DEKI_PACKAGE_VERSION
        return DEKI_PACKAGE_VERSION;
#else
        return "0.0.0-dev";
#endif
    }
    DEKI_PLUGIN_API const char* DekiPluginGetReflectionJson(void)
    {
        return "{}";
    }

    DEKI_PLUGIN_API int DekiPluginInit(void)
    {
        return 0;
    }

    DEKI_PLUGIN_API void DekiPluginShutdown(void)
    {
        s_JsonRegistered = false;
    }

#ifdef DEKI_EDITOR
    DEKI_PLUGIN_API int DekiPluginGetComponentCount(void)
    {
        return DekiJsonGetAutoComponentCount();
    }
    DEKI_PLUGIN_API const Deki::ComponentMeta* DekiPluginGetComponentMeta(int index)
    {
        return DekiJsonGetAutoComponentMeta(index);
    }
#else
    DEKI_PLUGIN_API int DekiPluginGetComponentCount(void)
    {
        return 0;
    }
    DEKI_PLUGIN_API const Deki::ComponentMeta* DekiPluginGetComponentMeta(int)
    {
        return nullptr;
    }
#endif

    DEKI_PLUGIN_API void DekiPluginRegisterComponents(void)
    {
#ifdef DEKI_EDITOR
        DekiJsonEnsureRegistered();
#endif
    }

}  // extern "C"
