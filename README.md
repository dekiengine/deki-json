# Deki JSON

Documentation: https://dekiengine.github.io/deki-json/ (components and properties, generated from the code)

JSON parsing and serialization for the Deki Engine, wrapping a vendored copy of [cJSON](https://github.com/DaveGamble/cJSON) behind the `DekiJson` facade.

Part of the [Deki Engine](https://github.com/dekiengine/deki-engine) module ecosystem.

## Namespace

This package's types live in `DekiJson`. Scene files store the qualified
name, so a component is `DekiJson::SomeComponent` there, and code naming one
needs the namespace:

```cpp
using namespace DekiJson;
obj->AddComponent<SomeComponent>();
```

Scenes saved before 0.16.0 used bare names and still load: every component
records what it used to be called, and a save writes the current name.

## Installation

Install via the Package Manager inside the Deki Editor, or `DekiEditor --packages-add deki-json <project>`.

## Dependencies

| Dependency | Type |
|---|---|
| cJSON | External, vendored under `third_party/cJSON` (MIT) |

## License

Licensed under the Apache License, Version 2.0. See [LICENSE](LICENSE) for details.

Third-party licenses are listed in [NOTICE](NOTICE).
