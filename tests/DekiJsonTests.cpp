// Document's contract: parsing and the nesting limit that keeps a device's
// stack safe, typed reads with their fallbacks, borrowed views versus owned
// roots, building and serialising.
//
// Ownership is where this goes wrong quietly. A borrowed view linked into a
// second tree as-is would be freed by both roots; a moved-from Document that
// still owned its node would free it twice. Those are crashes at shutdown, far
// from the line that caused them, so they are pinned here.

#include <gtest/gtest.h>

#include <DekiJson.h>

#include <string>
#include <utility>

using namespace DekiJson;

namespace
{

std::string Nested(int depth)
{
    return std::string(depth, '[') + std::string(depth, ']');
}

}  // namespace

// --- Parsing ---

TEST(DekiJson, ParsesAnObjectAndReadsItsMembers)
{
    Document d = Document::Parse(R"({"n": 2.5, "b": true, "s": "hi"})");
    ASSERT_TRUE(d.Valid());
    EXPECT_DOUBLE_EQ(d.GetNumber("n"), 2.5);
    EXPECT_TRUE(d.GetBool("b"));
    EXPECT_EQ(d.GetString("s"), "hi");
    EXPECT_EQ(d.Size(), 3);
}

TEST(DekiJson, MalformedTextIsInvalid)
{
    EXPECT_FALSE(Document::Parse("{\"a\": ").Valid());
    EXPECT_FALSE(Document::Parse("").Valid());
    EXPECT_FALSE(Document::Parse("not json").Valid());
}

TEST(DekiJson, NestingIsLimitedTo32)
{
    // The header promises this limit: deeper input would recurse the parser
    // past what a device's task stack holds.
    EXPECT_TRUE(Document::Parse(Nested(32)).Valid());
    EXPECT_FALSE(Document::Parse(Nested(33)).Valid());
    EXPECT_FALSE(Document::Parse(Nested(500)).Valid());
}

// --- Reading ---

TEST(DekiJson, MissingOrMistypedMembersGiveTheFallback)
{
    Document d = Document::Parse(R"({"n": "not a number", "b": 1, "s": 7})");
    ASSERT_TRUE(d.Valid());
    EXPECT_DOUBLE_EQ(d.GetNumber("n", -1.0), -1.0);
    EXPECT_TRUE(d.GetBool("b", true));    // 1 is a number, not a bool
    EXPECT_FALSE(d.GetBool("b", false));
    EXPECT_EQ(d.GetString("s", "fb"), "fb");
    EXPECT_DOUBLE_EQ(d.GetNumber("absent", 4.0), 4.0);
    EXPECT_FALSE(d.HasKey("absent"));
    EXPECT_TRUE(d.HasKey("n"));
}

TEST(DekiJson, AnInvalidDocumentReadsAsEmpty)
{
    Document d;
    EXPECT_FALSE(d.Valid());
    EXPECT_FALSE(d.HasKey("a"));
    EXPECT_DOUBLE_EQ(d.GetNumber("a", 3.0), 3.0);
    EXPECT_EQ(d.GetString("a", "x"), "x");
    EXPECT_EQ(d.GetString("a", nullptr), "");
    EXPECT_EQ(d.Size(), 0);
    EXPECT_FALSE(d.GetChild("a").Valid());
    EXPECT_FALSE(d.GetAt(0).Valid());
    EXPECT_EQ(d.Dump(), "");
}

TEST(DekiJson, NullKeysAreRefused)
{
    Document d = Document::Parse(R"({"a": 1})");
    EXPECT_FALSE(d.HasKey(nullptr));
    EXPECT_DOUBLE_EQ(d.GetNumber(nullptr, 9.0), 9.0);
    d.SetNumber(nullptr, 1.0);
    EXPECT_EQ(d.Dump(), R"({"a":1})");
}

TEST(DekiJson, ArraysIndexInRangeOnly)
{
    Document d = Document::Parse(R"([10, 20, 30])");
    ASSERT_TRUE(d.Valid());
    EXPECT_EQ(d.Size(), 3);
    EXPECT_TRUE(d.GetAt(0).Valid());
    EXPECT_TRUE(d.GetAt(2).Valid());
    EXPECT_FALSE(d.GetAt(3).Valid());
    EXPECT_FALSE(d.GetAt(-1).Valid());
}

TEST(DekiJson, ChildViewsReadNestedValues)
{
    Document d = Document::Parse(R"({"pos": {"x": 1, "y": 2}, "list": [{"id": 7}]})");
    Document pos = d.GetChild("pos");
    ASSERT_TRUE(pos.Valid());
    EXPECT_DOUBLE_EQ(pos.GetNumber("y"), 2.0);
    EXPECT_DOUBLE_EQ(d.GetChild("list").GetAt(0).GetNumber("id"), 7.0);
}

// --- Building ---

TEST(DekiJson, BuildsAndDumpsCompactText)
{
    Document d = Document::Object();
    d.SetNumber("n", 3);
    d.SetBool("ok", true);
    d.SetString("name", "deki");
    EXPECT_EQ(d.Dump(), R"({"n":3,"ok":true,"name":"deki"})");
}

TEST(DekiJson, SettingAKeyAgainReplacesIt)
{
    // A setter that appended instead would leave {"n":1,"n":2}: valid to
    // cJSON, but GetNumber would keep returning the first value.
    Document d = Document::Object();
    d.SetNumber("n", 1);
    d.SetNumber("n", 2);
    d.SetString("s", "a");
    d.SetString("s", "b");
    d.SetBool("b", false);
    d.SetBool("b", true);
    EXPECT_DOUBLE_EQ(d.GetNumber("n"), 2.0);
    EXPECT_EQ(d.GetString("s"), "b");
    EXPECT_TRUE(d.GetBool("b"));
    EXPECT_EQ(d.Size(), 3);
    EXPECT_EQ(d.Dump(), R"({"n":2,"s":"b","b":true})");
}

TEST(DekiJson, SetChildReplacesAMemberOfAnyType)
{
    Document d = Document::Object();
    d.SetNumber("c", 1);
    Document child = Document::Object();
    child.SetBool("on", true);
    d.SetChild("c", std::move(child));
    EXPECT_EQ(d.Dump(), R"({"c":{"on":true}})");
    d.SetString("c", "flat");
    EXPECT_EQ(d.Dump(), R"({"c":"flat"})");
}

TEST(DekiJson, StringsAreEscapedAndRoundTrip)
{
    Document d = Document::Object();
    d.SetString("s", "quote \" backslash \\ newline \n");
    Document back = Document::Parse(d.Dump());
    ASSERT_TRUE(back.Valid());
    EXPECT_EQ(back.GetString("s"), "quote \" backslash \\ newline \n");
}

TEST(DekiJson, SetChildMovesARoot)
{
    Document child = Document::Object();
    child.SetNumber("x", 1);
    Document parent = Document::Object();
    parent.SetChild("c", std::move(child));
    EXPECT_FALSE(child.Valid());  // NOLINT(bugprone-use-after-move): the contract under test
    EXPECT_EQ(parent.Dump(), R"({"c":{"x":1}})");
}

TEST(DekiJson, SetChildCopiesABorrowedView)
{
    // Linked as-is, the node would belong to both trees and be freed twice
    // when both roots go out of scope.
    Document source = Document::Parse(R"({"inner": {"v": 5}})");
    Document target = Document::Object();
    target.SetChild("copy", source.GetChild("inner"));
    EXPECT_EQ(source.Dump(), R"({"inner":{"v":5}})");
    EXPECT_EQ(target.Dump(), R"({"copy":{"v":5}})");
}

TEST(DekiJson, PushBackMovesRootsAndCopiesViews)
{
    Document source = Document::Parse(R"([{"id": 1}])");
    Document arr = Document::Array();
    Document item = Document::Object();
    item.SetNumber("id", 2);
    arr.PushBack(std::move(item));
    arr.PushBack(source.GetAt(0));
    EXPECT_FALSE(item.Valid());  // NOLINT(bugprone-use-after-move): the contract under test
    EXPECT_EQ(arr.Size(), 2);
    EXPECT_EQ(arr.Dump(), R"([{"id":2},{"id":1}])");
    EXPECT_EQ(source.Dump(), R"([{"id":1}])");
}

TEST(DekiJson, InvalidChildrenAreIgnored)
{
    Document d = Document::Object();
    d.SetChild("c", Document());
    Document arr = Document::Array();
    arr.PushBack(Document());
    EXPECT_EQ(d.Dump(), "{}");
    EXPECT_EQ(arr.Size(), 0);
}

// --- Ownership ---

TEST(DekiJson, MoveTransfersOwnership)
{
    Document a = Document::Parse(R"({"k": 1})");
    Document b = std::move(a);
    EXPECT_FALSE(a.Valid());  // NOLINT(bugprone-use-after-move): the contract under test
    EXPECT_TRUE(b.Valid());

    Document c = Document::Object();
    c = std::move(b);
    EXPECT_DOUBLE_EQ(c.GetNumber("k"), 1.0);
}
