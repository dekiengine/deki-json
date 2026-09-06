#include "DekiJsonPackage.h"
#include <deki/interop/Plugin.h>
#include <deki/LogSystem.h>

#ifdef DEKI_EDITOR
extern void DekiJson_RegisterComponents();
extern int  DekiJson_GetAutoComponentCount();
extern const Deki::ComponentMeta* DekiJson_GetAutoComponentMeta(int index);
#endif

static bool s_JsonRegistered = false;

extern "C" {

DEKI_JSON_API int DekiJson_EnsureRegistered(void)
{
#ifdef DEKI_EDITOR
    if (s_JsonRegistered) return DekiJson_GetAutoComponentCount();
    s_JsonRegistered = true;
    DekiJson_RegisterComponents();
    return DekiJson_GetAutoComponentCount();
#else
    return 0;
#endif
}

DEKI_PLUGIN_API const char* DekiPlugin_GetName(void)    { return "Deki JSON Package"; }
DEKI_PLUGIN_API const char* DekiPlugin_GetVersion(void)
{
#ifdef DEKI_PACKAGE_VERSION
    return DEKI_PACKAGE_VERSION;
#else
    return "0.0.0-dev";
#endif
}
DEKI_PLUGIN_API const char* DekiPlugin_GetReflectionJson(void) { return "{}"; }

DEKI_PLUGIN_API int  DekiPlugin_Init(void)
{
    DEKI_LOG_INFO("[deki-json] DekiPlugin_Init");
    return 0;
}

DEKI_PLUGIN_API void DekiPlugin_Shutdown(void) { s_JsonRegistered = false; }

#ifdef DEKI_EDITOR
DEKI_PLUGIN_API int  DekiPlugin_GetComponentCount(void) { return DekiJson_GetAutoComponentCount(); }
DEKI_PLUGIN_API const Deki::ComponentMeta* DekiPlugin_GetComponentMeta(int index)
{
    return DekiJson_GetAutoComponentMeta(index);
}
#else
DEKI_PLUGIN_API int  DekiPlugin_GetComponentCount(void) { return 0; }
DEKI_PLUGIN_API const Deki::ComponentMeta* DekiPlugin_GetComponentMeta(int) { return nullptr; }
#endif

DEKI_PLUGIN_API void DekiPlugin_RegisterComponents(void)
{
#ifdef DEKI_EDITOR
    int n = DekiJson_EnsureRegistered();
    DEKI_LOG_INFO("[deki-json] DekiPlugin_RegisterComponents -> %d component(s)", n);
#endif
}


}  // extern "C"
