#pragma once

#include "DekiJsonPackage.h"
#include <string>

namespace DekiJson
{

/// Move-only JSON document.
///
/// A Document is either a root (from Parse, Object or Array), which owns its
/// tree, or a borrowed view of a child node from GetChild or GetAt. A
/// borrowed view is valid only while its root is alive.
///
/// Built on cJSON, which stays hidden: callers never include cJSON headers.
class DEKI_JSON_API Document
{
public:
    Document();
    ~Document();
    Document(Document&&) noexcept;
    Document& operator=(Document&&) noexcept;
    Document(const Document&) = delete;
    Document& operator=(const Document&) = delete;

    // --- Construction ---

    /// Parses JSON text. Returns an invalid Document (see Valid()) when the
    /// text does not parse, or when arrays or objects nest more than 32 deep,
    /// which would overflow a device's stack.
    static Document Parse(const std::string& text);

    /// A new, empty object or array root.
    static Document Object();
    static Document Array();

    bool Valid() const;

    // --- Reading (object access) ---

    bool HasKey(const char* key) const;
    double GetNumber(const char* key, double fallback = 0.0) const;
    bool GetBool(const char* key, bool fallback = false) const;
    std::string GetString(const char* key, const char* fallback = "") const;

    /// A borrowed view of the named child, invalid when there is none. The
    /// view is valid only while this Document is alive.
    Document GetChild(const char* key) const;

    // --- Reading (array access) ---

    /// Number of elements in an array or members in an object; 0 otherwise.
    int Size() const;

    /// A borrowed view of the element at `index`, invalid when out of range.
    Document GetAt(int index) const;

    // --- Building (object setters) ---

    void SetNumber(const char* key, double value);
    void SetBool(const char* key, bool value);
    void SetString(const char* key, const std::string& value);

    /// Moves `child` into this object under `key`; `child` is invalid
    /// afterwards. A borrowed view (from GetChild or GetAt) is copied, and
    /// its own tree stays as it was.
    void SetChild(const char* key, Document child);

    // --- Building (array push) ---

    /// Moves `child` to the end of this array; `child` is invalid afterwards.
    /// A borrowed view is copied, as with SetChild.
    void PushBack(Document child);

    // --- Serialization ---

    /// Compact JSON text with no whitespace. Empty on failure.
    std::string Dump() const;

private:
    // `m_Node` is a `cJSON*` stored as void* to keep cJSON out of this
    // header. `m_Owns` is true for roots and false for borrowed views.
    void* m_Node = nullptr;
    bool m_Owns = false;

    Document(void* node, bool owns)
        : m_Node(node),
          m_Owns(owns)
    {
    }
};

}  // namespace DekiJson
