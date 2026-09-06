#include "DekiJson.h"
#include <deki/LogSystem.h>

// cJSON is vendored under third_party/cJSON/ so this module has zero MCU-platform
// coupling. The .c file is picked up by the module's recursive source glob.
#include "third_party/cJSON/cJSON.h"

#include <cstring>

namespace Deki { namespace Json {

// ---- ctor / dtor / move ----

Document::Document() = default;

Document::~Document()
{
    if (m_Owns && m_Node) {
        cJSON_Delete(static_cast<cJSON*>(m_Node));
    }
    m_Node = nullptr;
    m_Owns = false;
}

Document::Document(Document&& other) noexcept
    : m_Node(other.m_Node), m_Owns(other.m_Owns)
{
    other.m_Node = nullptr;
    other.m_Owns = false;
}

Document& Document::operator=(Document&& other) noexcept
{
    if (this != &other) {
        if (m_Owns && m_Node) cJSON_Delete(static_cast<cJSON*>(m_Node));
        m_Node       = other.m_Node;
        m_Owns       = other.m_Owns;
        other.m_Node = nullptr;
        other.m_Owns = false;
    }
    return *this;
}

// ---- construction ----

Document Document::Parse(const std::string& text)
{
    cJSON* root = cJSON_Parse(text.c_str());
    if (!root) {
        DEKI_LOG_ERROR("[deki-json] parse failed");
        return Document();
    }
    return Document(root, /*owns=*/true);
}

Document Document::Object()
{
    return Document(cJSON_CreateObject(), /*owns=*/true);
}

Document Document::Array()
{
    return Document(cJSON_CreateArray(), /*owns=*/true);
}

bool Document::Valid() const
{
    return m_Node != nullptr;
}

// ---- reading ----

bool Document::HasKey(const char* key) const
{
    if (!m_Node || !key) return false;
    return cJSON_GetObjectItem(static_cast<cJSON*>(m_Node), key) != nullptr;
}

double Document::GetNumber(const char* key, double fallback) const
{
    if (!m_Node || !key) return fallback;
    cJSON* n = cJSON_GetObjectItem(static_cast<cJSON*>(m_Node), key);
    return cJSON_IsNumber(n) ? n->valuedouble : fallback;
}

bool Document::GetBool(const char* key, bool fallback) const
{
    if (!m_Node || !key) return fallback;
    cJSON* n = cJSON_GetObjectItem(static_cast<cJSON*>(m_Node), key);
    if (cJSON_IsBool(n)) return cJSON_IsTrue(n);
    return fallback;
}

std::string Document::GetString(const char* key, const char* fallback) const
{
    if (!m_Node || !key) return fallback ? fallback : "";
    cJSON* n = cJSON_GetObjectItem(static_cast<cJSON*>(m_Node), key);
    if (cJSON_IsString(n) && n->valuestring) return n->valuestring;
    return fallback ? fallback : "";
}

Document Document::GetChild(const char* key) const
{
    if (!m_Node || !key) return Document();
    cJSON* n = cJSON_GetObjectItem(static_cast<cJSON*>(m_Node), key);
    if (!n) return Document();
    return Document(n, /*owns=*/false);  // borrowed view
}

int Document::Size() const
{
    if (!m_Node) return 0;
    return cJSON_GetArraySize(static_cast<cJSON*>(m_Node));
}

Document Document::GetAt(int index) const
{
    if (!m_Node || index < 0) return Document();
    cJSON* n = cJSON_GetArrayItem(static_cast<cJSON*>(m_Node), index);
    if (!n) return Document();
    return Document(n, /*owns=*/false);
}

// ---- building ----

void Document::SetNumber(const char* key, double value)
{
    if (!m_Node || !key) return;
    cJSON_AddNumberToObject(static_cast<cJSON*>(m_Node), key, value);
}

void Document::SetBool(const char* key, bool value)
{
    if (!m_Node || !key) return;
    cJSON_AddBoolToObject(static_cast<cJSON*>(m_Node), key, value);
}

void Document::SetString(const char* key, const std::string& value)
{
    if (!m_Node || !key) return;
    cJSON_AddStringToObject(static_cast<cJSON*>(m_Node), key, value.c_str());
}

void Document::SetChild(const char* key, Document child)
{
    if (!m_Node || !key || !child.Valid()) return;
    cJSON_AddItemToObject(static_cast<cJSON*>(m_Node),
                          key,
                          static_cast<cJSON*>(child.m_Node));
    // Ownership transferred into the parent tree.
    child.m_Node = nullptr;
    child.m_Owns = false;
}

void Document::PushBack(Document child)
{
    if (!m_Node || !child.Valid()) return;
    cJSON_AddItemToArray(static_cast<cJSON*>(m_Node),
                         static_cast<cJSON*>(child.m_Node));
    child.m_Node = nullptr;
    child.m_Owns = false;
}

// ---- serialization ----

std::string Document::Dump() const
{
    if (!m_Node) return {};
    char* s = cJSON_PrintUnformatted(static_cast<cJSON*>(m_Node));
    if (!s) return {};
    std::string out(s);
    cJSON_free(s);
    return out;
}

}}  // namespace Deki::Json
