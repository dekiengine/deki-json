# Deki JSON

Docs: https://dekiengine.github.io/deki-json/ (components and properties, generated from the code)

JSON parsing and serialization for the Deki Engine, wrapping a vendored copy of [cJSON](https://github.com/DaveGamble/cJSON) behind the `DekiJson` facade.

Part of the [Deki Engine](https://github.com/dekiengine/deki-engine) module ecosystem.

## Namespace

Types live in `DekiJson`. Scene files store the qualified name, and so does code:

```cpp
using namespace DekiJson;
obj->AddComponent<SomeComponent>();
```

Scenes saved before 0.16.0 used bare names and still load; saving writes the current one.

## Install

Package Manager in the Deki Editor, or `DekiEditor --packages-add deki-json <project>`.

## Dependencies

| Dependency | Type |
|---|---|
| cJSON | External, vendored under `third_party/cJSON` (MIT) |

## License

Apache 2.0. See [LICENSE](LICENSE).

Third-party licenses are listed in [NOTICE](NOTICE).
