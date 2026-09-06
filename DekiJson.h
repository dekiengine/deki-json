#pragma once

#include "DekiJsonPackage.h"
#include <string>

namespace Deki { namespace Json {

/**
 * @brief Move-only JSON document.
 *
 * A Document is either a root (created by Parse / Object / Array) which owns
 * the underlying tree, or a borrowed view of a child node obtained via
 * GetChild / GetAt. Borrowed views remain valid only while the owning root
 * is alive — keep the root around.
 *
 * Backed by cJSON internally. Consumers never include cJSON headers — the
 * public API is portable across every platform this module is built for.
 */
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

    /// Parse JSON text. Returns an invalid Document on parse failure
    /// (check with Valid()).
    static Document Parse(const std::string& text);

    /// Build a new empty object/array root.
    static Document Object();
    static Document Array();

    bool Valid() const;

    // --- Reading (object access) ---

    bool   HasKey(const char* key) const;
    double GetNumber(const char* key, double fallback = 0.0) const;
    bool   GetBool  (const char* key, bool   fallback = false) const;
    std::string GetString(const char* key, const char* fallback = "") const;

    /// Returns a borrowed view of the named child. Invalid if missing or not
    /// an object/array. The view is only valid while this Document is alive.
    Document GetChild(const char* key) const;

    // --- Reading (array access) ---

    /// Number of elements (for arrays) or members (for objects). 0 otherwise.
    int Size() const;

    /// Borrowed view of the element at `index`. Invalid if out of range.
    Document GetAt(int index) const;

    // --- Building (object setters) ---

    void SetNumber(const char* key, double value);
    void SetBool  (const char* key, bool   value);
    void SetString(const char* key, const std::string& value);

    /// Move `child` into this object under `key`. `child` becomes invalid
    /// after the call.
    void SetChild(const char* key, Document child);

    // --- Building (array push) ---

    /// Move `child` to the end of this array. `child` becomes invalid.
    void PushBack(Document child);

    // --- Serialization ---

    /// Compact (no-whitespace) JSON text. Empty string on failure.
    std::string Dump() const;

private:
    // Hide the concrete cJSON dependency. `m_Node` is a `cJSON*` cast to
    // void*; `m_Owns` is true for roots (and false for borrowed children
    // returned by GetChild / GetAt).
    void* m_Node = nullptr;
    bool  m_Owns = false;

    Document(void* node, bool owns) : m_Node(node), m_Owns(owns) {}
};

}}  // namespace Deki::Json
