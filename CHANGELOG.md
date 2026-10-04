# Changelog

Notable changes to `deki-json`. Engine and editor changes are in the
[engine changelog](https://github.com/dekiengine/deki-engine/blob/master/CHANGELOG.md).

A package's `minEngine` names the engine version it needs. Before 1.0 a
breaking change bumps the minor across the editor, the engine and every
package together, so a package with no changes of its own is still released
alongside one that has them.

## Unreleased

### Changed
- **Names follow the code style** (deki-engine/docs/codestyle): types, functions and enum values are PascalCase, constants kPascalCase, members m_PascalCase, locals and parameters camelCase. The code is formatted with clang-format 22.
- The functions the editor finds by name are PascalCase: DekiJsonRegisterComponents, DekiJsonGetAutoComponentCount, DekiJsonEnsureRegistered and the rest. Built against engine ABI 21; a build of this package from before does not load and is rebuilt.

### Fixed
- `SetChild` and `PushBack` given a borrowed view (from `GetChild` or `GetAt`)
  linked a node that still belonged to its own tree, so both roots freed it
  and the program crashed. The view is now copied, and its tree is left as it
  was.
- Arrays and objects nested more than 32 deep no longer parse (cJSON's own
  limit was 1000). Each level takes stack, and a few hundred overflowed a
  device's 16 KB task stack: JSON from the network could crash the board.
  The limit is the same on every platform.

## 0.17.0

### Changed
- `minEngine` 0.17.0. Reflection ABI 20: the package must be rebuilt.

## 0.16.0

### Changed
- **Moved into the `DekiJson` namespace.** Every component was declared at global
  scope, which made its identity a bare class name — the name a scene file
  stores and the name the registry keys on — so two packages defining one name
  collided there with nothing to tell them apart. Each component carries
  `DEKI_FORMER_NAME` with the name it was saved under before, so existing
  scenes load unchanged and are written back qualified on the next save.
  Code naming these types needs the namespace: `using namespace DekiJson;` or a
  qualified name.
- Enum properties are stored by name rather than by number, so appending to an
  enum or reordering one no longer changes what a saved scene means. Files
  written before this still read.
- `minEngine` 0.16.0. Reflection ABI 17: the package must be rebuilt.

## 0.15.0

### Changed
- No changes of its own. Released alongside engine 0.15.0 so `minEngine`
  tracks the engine version.
