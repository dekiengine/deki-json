#pragma once

#ifdef _WIN32
    #ifdef DEKI_JSON_EXPORTS
        #define DEKI_JSON_API __declspec(dllexport)
    #else
        #define DEKI_JSON_API __declspec(dllimport)
    #endif
#else
    #define DEKI_JSON_API __attribute__((visibility("default")))
#endif
