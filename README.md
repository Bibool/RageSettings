# Rage Settings

A modular game-settings framework for Unreal Engine: four settings categories (Game / Audio / Video /
Input), a pending-vs-current dirty-tracking model shared by all of them, and a UMG layer that can
generate its own rows by reflection so most new settings need zero UI work.

Video settings cover the usual display/scalability/post-process surface plus DLSS, DLSS Frame
Generation, DLSS Ray Reconstruction, FSR (+ frame interpolation), XeSS (+ XeFG), Reflex and XeLL,
each gated behind a runtime support query so unsupported options never reach the player.

Built and tested against **Unreal Engine 5.7**. MIT licensed — see [LICENSE](LICENSE).

---

## Contents

- [Modules](#modules)
- [Requirements](#requirements)
- [Installation](#installation)
- [How it works](#how-it-works)
- [Using each type of setting](#using-each-type-of-setting)
  - [Toggle (bool)](#toggle-bool)
  - [Slider (numeric)](#slider-numeric)
  - [Selection (enum)](#selection-enum)
  - [Combo (drop-down)](#combo-drop-down)
  - [Keybinds](#keybinds)
  - [Video settings](#video-settings)
- [Adding your own settings](#adding-your-own-settings)
- [Building the UI](#building-the-ui)
- [Runtime API](#runtime-api)
- [Localization](#localization)
- [Where things are saved](#where-things-are-saved)
- [Limitations](#limitations)
- [Third-party software](#third-party-software)
- [License](#license)

---

## Modules

| Module | Type | What's in it |
|---|---|---|
| `RageSettings` | Runtime | Settings categories, the subsystem, developer settings, reflection helpers. No UMG dependency. |
| `RageSettingsUI` | Runtime | Row widgets, panels, the settings view, modals, UI developer settings. |
| `RageSettingsShared` | Runtime | `LogRageSettings` / the `S_LOG` macro, shared by both. |

Add `RageSettings` to your module's dependencies to talk to settings from gameplay code; add
`RageSettingsUI` as well if you subclass the widgets.

```csharp
PublicDependencyModuleNames.AddRange(new string[] { "RageSettings", "RageSettingsUI" });
```

---

## Requirements

**Always required**

- **Enhanced Input**, with `bEnableUserSettings = true` — the whole keybind category is a thin layer
  over `UEnhancedInputUserSettings`, and without user settings enabled every remap call no-ops.
- **`URageVideoSettings` registered as the engine's `GameUserSettings` class.** The subsystem casts
  `GEngine->GetGameUserSettings()` to `URageVideoSettings` during `Initialize()`; if that cast fails
  it logs an error and **every category is left uninitialized**, not just Video.
- **GameplayTags** — keybind categories are identified by tag.
- A **`UGameInstance`** — the subsystem is a `UGameInstanceSubsystem`, and input remapping resolves
  through local player 0.

**Optional — the upscaler SDKs.** Each is independent, and **which of them ship is the game's
decision, not the plugin's.** Enable the ones you want in your `.uproject`; the plugin links whatever
it finds and compiles out the rest. With an SDK absent, its `Is*Supported()` returns `false`, the
option is never offered in the UI, and a persisted setting that names it falls back to TSR on apply.

| Feature | Plugin | Compile guard | Defined by |
|---|---|---|---|
| DLSS / DLSS-RR | `DLSS` | `WITH_DLSS` | `DLSSBlueprint` (per-platform) |
| DLSS Frame Generation | `StreamlineDLSSG` (+ `Streamline`) | `WITH_STREAMLINE` | `StreamlineBlueprint` |
| FSR / frame interpolation | `FSR` | `WITH_FSR` | `RageSettings.Build.cs` |
| XeSS | `XeSS` | `WITH_XESS` | `XeSSBlueprint` |
| XeSS Frame Generation (XeFG) | `XeSS` | `WITH_XEFG` | `XeFGBlueprint` |
| XeLL | `XeSS` | `WITH_XELL` | `XeLLBlueprint` |

The wiring, if you need to add a vendor or drop one:

- `RageSettings.uplugin` lists all four as `"Optional": true` — enabled when present, no hard error
  when absent.
- `RageSettings.Build.cs` probes for each plugin directory alongside its own and only then adds that
  vendor's modules to `PrivateDependencyModuleNames`. The probe assumes vendor plugins sit as
  siblings under `Plugins/`; widen it if yours live in the engine or a Marketplace folder.
- **Each `WITH_*` guard must be defined in exactly one place, and which place depends on presence.**
  When the SDK is there, its own Blueprint module publishes the guard (often per-platform) — defining
  it again in `RageSettings.Build.cs` is a redefinition, which is why forcing `WITH_DLSS=1` was
  wrong. When the SDK is absent nobody publishes it, so the `else` branch must define it to `0`: UE
  compiles with `/Wundef` promoted to an error (C4668), so an undefined macro in an `#if` is a hard
  compile failure, **not** a silent 0. `WITH_FSR` is the exception on both counts — the FSR plugin
  publishes nothing, so we set it in both branches.

If you add a vendor, follow that shape: modules in the `if`, `WITH_YOURS=0` in the `else`. Verify by
temporarily moving the plugin out of `Plugins/`, dropping its entry from the `.uproject`, and
rebuilding — a missing `=0` shows up immediately as C4668 on every `#if` that uses it.

The vendor modules cannot move to your game module: the code that calls them
(`RageVideoSettings.cpp`) lives in `RageSettings`, and UBT dependencies follow includes. Decoupling
them properly would mean an upscaler-backend interface the game implements and registers.

Ray tracing options are gated on `GRHISupportsRayTracing`, so they need an RHI/hardware combination
that supports it (DX12 + SM6).

---

## Installation

1. Copy `RageSettings/` into your project's `Plugins/` folder, then enable it in the Editor (or add
   `"RageSettings"` to your `.uproject`'s `Plugins` array). If you want the upscalers, drop those
   plugins into `Plugins/` too and enable them in your `.uproject` — RageSettings picks up whichever
   are present and needs none of them.

2. Point the engine at `URageVideoSettings` so it becomes *the* `GameUserSettings`, in
   `Config/DefaultEngine.ini`:

   ```ini
   [/Script/Engine.Engine]
   GameUserSettingsClassName=/Script/RageSettings.RageVideoSettings
   ```

3. Make sure Enhanced Input is enabled and that any `UInputAction` you want remappable has its
   mapping's **Setting Behavior** set to **Override Settings** with a unique name in its Input
   Mapping Context — that name is the `MappingName` `RemapPlayerKey()` expects.

4. `bEnableUserSettings` must be true in **Project Settings → Enhanced Input**:

   ```ini
   [/Script/EnhancedInput.EnhancedInputDeveloperSettings]
   bEnableUserSettings=True
   ```

5. Your IMC containing the rebindable actions must be **registered with the Enhanced Input user
   settings** — either register it manually
   (`UEnhancedInputUserSettings::RegisterInputMappingContext`), or tick "Register with User Settings"
   in the config when adding the IMC to the subsystem. Until it is registered the active key profile
   has no row for those mappings, so `GetCurrentKeyForMapping()` returns an invalid key and conflict
   detection can't see them.

6. List the actions you want shown in the keybind UI under **Project Settings → Rage - Settings →
   Rage|Input → Remappable Actions** (see [Keybinds](#keybinds)).

7. Configure **Project Settings → Rage - Settings UI** — at minimum the default row widget classes
   (they default to the widgets shipped in the plugin's Content folder) and the keybind category
   display config.

---

## How it works

Every category implements `IRageSettingsCategoryInterface` and holds three instances of itself:

```
Current   this            what is live and what gets written to disk
Pending   GetPendingSettings()   what the UI edits
Defaults  GetDefaultSettings()   the CDO snapshot, taken at load
```

Only `UPROPERTY(Config)` fields participate — `RageSettings::AreObjectsEqual` /
`CopyObjectProperties` iterate the class's config properties, so **dirty tracking, apply, save,
revert and reset all pick up new fields automatically the moment you mark them `Config`.**

```
LoadSettings()          Current from disk, Pending = Current, Defaults = CDO. Once, at startup.
   ... UI edits Pending ...
IsDirty()               Pending != Current. Always derived, never a cached bool.
ApplySettings()         Pending -> Current, then do the real engine-side work.
SaveSettings()          Persist Current.
ResetToDefault()        Pending = Defaults. Does not apply or save on its own.
RevertPendingChanges()  Pending = Current. The "Cancel" path.
```

`URageSettingsSubsystem` owns all four, drives them together (`ApplyAllDirtySettings`,
`RevertAllPendingChanges`, …) and re-broadcasts each category's dirty transitions through
`AnyCategoryDirtyStateChangedDelegate`. **Query the subsystem, not the settings objects directly.**

---

## Using each type of setting

The UI never hard-codes a row per field. `URageSettingsRowGeneratorPanelBase::BuildRows()` walks the
Pending object's class — base class fields first, subclass fields after — and spawns one row per
property that is **both `Config` and `EditAnywhere`**:

```cpp
UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Rage|Game")
bool bShowDamageNumbers = true;
```

| Property type | Row generated | Widget class used |
|---|---|---|
| `bool` | Toggle | `DefaultToggleRowClass` |
| `float` / `double` / `int32` / `int64` / plain `uint8` | Slider | `DefaultSliderRowClass` |
| `enum class` / `TEnumAsByte` | Selection | `DefaultSelectionRowClass` |
| anything else (structs, arrays, `FString`, object refs) | — skipped | — |

Two rules follow from this:

- **`Config` without `EditAnywhere`** → the field is tracked, applied, saved and reset, but gets no
  row. This is how `URageVideoSettings` keeps its hand-authored panel.
- **`EditAnywhere` without `Config`** → no row *and* no persistence. Not useful.

Per-row overrides come from `GetRowDescriptors()` (label, clamp range, slider format) and from
`RowWidgetClassOverrides` in **Rage - Settings UI** (a per-property-name widget class override; a
class that doesn't derive from the expected row base is ignored with a warning).

### Toggle (bool)

```cpp
UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Rage|Audio")
bool bSubtitlesEnabled = false;
```

Nothing else to do — a toggle row appears with the label derived from the property name
(`bSubtitlesEnabled` → "Subtitles Enabled"). Add a descriptor entry only if you want a different or
localized label.

**Widget contract** — a Blueprint deriving `URageToggleRow` needs:

| Bind | Type | Required |
|---|---|---|
| `Label` | `UTextBlock` | yes (inherited from `URageRowBaseUserWidget`) |
| `CheckBox` | `UCheckBox` | yes |

### Slider (numeric)

```cpp
UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Rage|Game")
float DialogueSpeed = 1.f;
```

A raw numeric property gives you a slider over `[0, 1]` in `Raw` format, which is rarely what you
want. Override `GetRowDescriptors()` on your panel to set the range and readout:

```cpp
TArray<FRageSettingsRowDescriptor> UMyGameSettingsPanel::GetRowDescriptors() const
{
    FRageSettingsRowDescriptor DialogueSpeed;
    DialogueSpeed.PropertyName = GET_MEMBER_NAME_CHECKED(UMyGameSettings, DialogueSpeed);
    DialogueSpeed.Label        = RAGE_LOC("DialogueSpeed");
    DialogueSpeed.ClampMin     = 0.5f;
    DialogueSpeed.ClampMax     = 2.0f;
    DialogueSpeed.SliderFormat = ERageSliderDisplayFormat::Multiplier;

    return { DialogueSpeed };
}
```

`ERageSliderDisplayFormat`: `Raw` (`1.00`), `Percent` (`100%`), `Multiplier` (`1.00x`), `Integer`
(`1`). Override `RefreshValueText` in Blueprint for anything more exotic (the shipped frame-rate row
does this to show "Uncapped" at 0).

Note the descriptor's clamp only bounds the **slider**; it is not a value clamp on the property. If a
value must be clamped everywhere, add a typed `SetPendingX()` on your settings class that clamps —
that's what `SetPendingMouseSensitivity` and `SetPendingFieldOfView` do.

**Widget contract** — Blueprint deriving `URageSliderRow`:

| Bind | Type | Required |
|---|---|---|
| `Label` | `UTextBlock` | yes |
| `Slider` | `USlider` | yes |
| `ValueText` | `UTextBlock` | optional |

### Selection (enum)

Enums become a left/right cycler with a pip strip — the console-style control, not a drop-down.

```cpp
UENUM(BlueprintType)
enum class EMyAimAssist : uint8 { Off, Light, Standard, Strong };

UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Rage|Game")
EMyAimAssist AimAssist = EMyAimAssist::Standard;
```

All enumerators are offered in declaration order (a trailing `_MAX` is dropped). Option labels come
from the enum's display names. To hide values or reorder them, don't use the generator for that
field — mark the property `Config` only and hand-author the row, calling
`SetOptions(RageSettingsUI::BuildEnumOptionsTexts<EMyAimAssist>({ ... }))` with just the values you
want.

`bWrapAround` (does the last option cycle back to the first) and `PipStyleOverride` are set on the
row Blueprint. Pip styles: `Cumulative` fills every pip up to the selection (quality levels),
`Progressive` fills only the selected one (unordered choices).

**Widget contract** — Blueprint deriving `URageSelectionRow`:

| Bind | Type | Required |
|---|---|---|
| `Label` | `UTextBlock` | yes |
| `Left_Button` | `UButton` | yes |
| `Right_Button` | `UButton` | yes |
| `PipView` | `URagePipView` | yes |
| `ValueText` | `UTextBlock` | optional |

`URagePipView` needs a `PipContainer` (`UPanelWidget`) bind and a `PipElementClass`. Each
`URagePipElement` is told its index and state (`Empty` / `Filled` / `Current`) and draws itself in the
`OnPipStateChanged` Blueprint event; `OnSlotAssigned` fires with its slot so you can set padding.

### Combo (drop-down)

`URageComboRow` wraps a `UComboBoxString`. **The row generator never produces one** — enums always
become Selection rows — so combos are for hand-authored rows whose options come from runtime data
rather than from a `UPROPERTY`. The resolution row is the canonical case: its options are whatever
`GetSupportedResolutions()` reports on this machine.

```cpp
ResolutionRow->SetLabel(RAGE_LOC("Resolution"));
ResolutionRow->SetOptions(Labels);                 // TArray<FString>
ResolutionRow->ValueChangedDelegate.AddUObject(this, &UMyPanel::HandleResolutionChanged);
ResolutionRow->SetSelectedIndex(Index);            // bNotify defaults to false
```

You can still get a combo for a generated row: point that property at a `URageComboRow` Blueprint via
`RowWidgetClassOverrides` in **Rage - Settings UI**.

**Widget contract** — Blueprint deriving `URageComboRow`:

| Bind | Type | Required |
|---|---|---|
| `Label` | `UTextBlock` | yes |
| `ComboBox` | `UComboBoxString` | yes |

### Keybinds

Keybinds are **not** part of the pending/apply cycle. A remap is written straight to the Enhanced
Input user settings and saved immediately, so the Apply button and the dirty markers don't reflect
keybind edits.

Requirements, in order:

1. `bEnableUserSettings=True` under `[/Script/EnhancedInput.EnhancedInputDeveloperSettings]`.
2. Each remappable `UInputAction` mapping in the IMC has **Setting Behavior = Override Settings** with
   a unique **name**. That name is the `MappingName`.
3. The IMC is registered with the user settings (manually, or via the config when adding the IMC).
4. Each action is listed in **Project Settings → Rage - Settings → Rage|Input → Remappable Actions**:

   ```ini
   [/Script/RageSettings.RageSettingsDeveloperSettings]
   +RemappableActions=(MappingName="Fire",DisplayName=NSLOCTEXT("[/Script/RageSettings]","Fire","Primary Fire"),Category=(TagName="Input.Category.Combat"),Priority=0)
   ```

   | Field | Meaning |
   |---|---|
   | `MappingName` | Must match the IMC mapping's name exactly. Logged as an error on edit if empty. |
   | `DisplayName` | Row label. Falls back to the mapping name in conflict messages if empty. |
   | `Category` | Gameplay tag grouping the row under a header. |
   | `Priority` | Order within the category, descending. |

5. Each category tag gets a header entry in **Rage - Settings UI → Rage|Input → Keybind Categories
   Config** (`Priority` orders the categories descending, `CategoryDisplayName` is the header text,
   `OptionalCategoryWidget` overrides `DefaultCategoryWidgetClass` for that one header). A tag with no
   entry still lists its actions, just without a header, and logs a warning.

Runtime behaviour: clicking the row's remap button listens for the next key or mouse button (Escape
cancels, focus loss cancels). If the chosen key is already used, `KeybindConflictModal` — if the
panel binds one — asks first; confirming **clears** the conflicting action's binding (it shows as
unbound), cancelling restores the row. Without a bound modal the rebind just goes through and the
other action is left unbound silently.

**Widget contract** — Blueprint deriving `URageKeybindRow` (note this derives `UUserWidget`
directly, not `URageRowBaseUserWidget`):

| Bind | Type | Required |
|---|---|---|
| `Label` | `UTextBlock` | yes |
| `RemapButton` | `UButton` | yes |
| `KeyText` | `UTextBlock` | yes |
| `ResetButton` | `UButton` | optional (per-row "reset to default") |

C++/Blueprint API on `URageInputSettings`: `RemapPlayerKey`, `GetCurrentKeyForMapping`,
`FindMappingsUsingKey`, `ClearKeyMapping`, `ResetKeyMappingToDefault`, `ResetAllKeyMappingsToDefault`.

### Video settings

`URageVideoSettingsPanel` is hand-authored rather than generated, because most of its rows need
runtime capability data no `UPROPERTY` can express — the monitor's resolution list, whether DLSS is
supported on this GPU, which DLSS modes the driver reports. Accordingly `URageVideoSettings`'
properties are `Config` but **not** `EditAnywhere`.

Every row on the panel is `BindWidgetOptional`: **bind only the rows you want to ship.** Don't want a
foliage slider or XeLL? Leave those widgets out of your Blueprint and the code skips them.

Interactions worth knowing before you drop rows:

- **Quality preset ↔ scalability.** Picking Low/Medium/High/Epic writes that level into all ten
  scalability fields; changing any single scalability row flips the preset to `Custom`.
- **Ray tracing master.** `RayTracing.bEnabled` gates the five sub-toggles — with it off, every RT
  cvar is pushed as 0 regardless of the sub-values, and the sub-rows are disabled.
- **Upscaler method** (`Off` / `TAAU` / `TSR` / `FSR` / `DLSS` / `XeSS`) selects which sub-block is
  relevant; only the supported methods are listed. A persisted method that is no longer supported
  falls back to TSR on apply, with a warning.
- **DLSS Ray Reconstruction** requires DLSS selected *and* ray tracing active *and*
  `IsDLSSRRSupported()`.
- **Frame generation** (DLSS-G and XeFG) is applied one tick late, deliberately — applying it in the
  same frame as a resolution change crashes.
- **HDR.** `IsHDRSupported()` reports the OS/monitor state at process start; UE cannot turn HDR on for
  the monitor, so a player who enables HDR in Windows must restart the game before the option becomes
  available. Only 1000 and 2000 nits are offered because that's what the engine maps.
- **Brightness** is `r.Gamma`, clamped to `BrightnessMin`/`BrightnessMax` from **Rage - Settings**.
- **Frame rate limit** of `0` means uncapped.

Global ranges used by the video and input rows live in **Project Settings → Rage - Settings**:
`FieldOfViewMin/Max`, `BrightnessMin/Max`, `FpsMax`, `SensitivityMin/Max`.

Audio needs one piece of setup there too: `MasterSoundMix` and `MasterSoundClass`. Without both,
`MasterVolume` is tracked and saved but never reaches the audio engine. (`bMuteWhenUnfocused` works
without them — it drives the transient primary volume directly.)

---

## Adding your own settings

**Game / Audio / Input** — subclass, add `Config` + `EditAnywhere` fields, and point the developer
settings at your class:

```cpp
UCLASS()
class UMyGameSettings : public URageGameSettings
{
    GENERATED_BODY()

public:
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "My|Game")
    bool bAutoThruster = true;

    virtual void ApplySettings() override
    {
        Super::ApplySettings();   // copies Pending -> Current and broadcasts
        // react to the new values here
    }
};
```

```ini
[/Script/RageSettings.RageSettingsDeveloperSettings]
GameSettingsClass=/Script/CoreUObject.Class'/Script/MyGame.MyGameSettings'
```

The Game and Audio panels need no changes at all — the new field shows up as a row on its own.
Reach it from code with the typed getter:

```cpp
if (const UMyGameSettings* Settings = URageSettingsSubsystem::Get(this)->GetTypedGameSettings<UMyGameSettings>())
{
    UseThruster(Settings->bAutoThruster);
}
```

Read **Current** (the settings object itself) from gameplay code, never `Pending` — Pending is the
UI's scratch copy.

**Video** — same idea one level deeper: subclass `URageVideoSettings` and point
`GameUserSettingsClassName` at your subclass instead. Config fields you add are dirty-tracked,
applied, saved and reset automatically, but will **not** appear in the UI until you add a row for
them in your own video panel subclass — the video panel is hand-authored.

If you'd rather generate rows for your video additions, subclass
`URageSettingsRowGeneratorPanelBase` for a second panel and call `BuildRows(Container,
VideoSettings->GetPendingSettings())` — the generator works against any category's Pending object.

---

## Building the UI

`URageSettingsView` is the top-level window: a tab strip driving a `UWidgetSwitcher`, plus a shared
Apply / Reset to Defaults / Close footer. **Apply is global** (every dirty category, not just the
visible tab); **Reset to Defaults is scoped to the active tab.**

The view does not create or remove itself. Whoever spawns it calls nothing extra — `NativeConstruct`
initializes every panel — but must:

- call `RequestClose()` instead of removing the widget, so un-applied edits get caught by the
  unsaved-changes prompt, and
- bind `ViewClosedDelegate` to know when it's actually safe to collapse / remove / pop it.

**Widget contract** — Blueprint deriving `URageSettingsView`:

| Bind | Type | Required |
|---|---|---|
| `CategorySwitcher` | `UWidgetSwitcher` | yes |
| `GamePanel` / `AudioPanel` / `VideoPanel` / `InputPanel` | matching panel classes | yes |
| `GameTabButton` / `AudioTabButton` / `VideoTabButton` / `InputTabButton` | `UButton` | yes |
| `GameTabDirtyMarker` / `AudioTabDirtyMarker` / `VideoTabDirtyMarker` / `InputTabDirtyMarker` | `UWidget` | yes |
| `ApplyButton` / `ResetToDefaultsButton` / `CloseButton` | `UButton` | yes |
| `UnsavedChangesModal` | `URageUnsavedChangesModal` | optional (without it, closing discards nothing and prompts nothing) |

`DefaultCategory` (EditDefaultsOnly) picks the tab shown first.

Panels:

| Panel | Required binds |
|---|---|
| `URageGameSettingsPanel` | `RowsContainer` (`UPanelWidget`) |
| `URageAudioSettingsPanel` | `RowsContainer` |
| `URageInputSettingsPanel` | `RowsContainer`, `KeybindListContainer`; `KeybindConflictModal` optional; `KeybindRowClass` set |
| `URageVideoSettingsPanel` | `ScalabilityContainer` + `ScalabilityRowClass`; every other row optional |

Modals derive `URageModalBase` (`Open` / `Close` / `IsOpen` / `SetMessage`, with the
`OnModalOpenStateChanged` Blueprint event for animations; `MessageText` bind is optional):

- `URageConfirmModal` — binds `ConfirmButton`, `CancelButton`. Generic two-choice prompt; used for
  keybind conflicts but not keybind-specific.
- `URageUnsavedChangesModal` — binds `ApplyAndCloseButton`, `DiscardAndCloseButton`, `CancelButton`.

The plugin ships working Blueprints for all of these under `/RageSettings/` (`W_Settings_View`,
`W_ToggleRow_View`, `W_SliderRow_View`, `W_ComboRow_VIew`, `W_SelectionRow_View`, `W_KeybindRow_View`,
`W_Category_View`, `W_Pip_View`, `W_PipElement`, `W_Confirm_View`, `W_UnsavedChanges_View`, and the
four panel widgets). They're plain starting points — reparent or replace them.

---

## Runtime API

```cpp
URageSettingsSubsystem* Settings = URageSettingsSubsystem::Get(WorldContextObject);
```

| Function | Notes |
|---|---|
| `GetVideoSettings()` / `GetAudioSettings()` / `GetGameSettings()` / `GetInputSettings()` | Blueprint-exposed. C++ has templated `GetTyped*Settings<T>()` for subclasses. |
| `ApplyAllDirtySettings()` | Applies each dirty category, broadcasting `CategoryAppliedDelegate` per category. |
| `SaveAllSettings()` | Persists every category, dirty or not. |
| `ApplyAndSaveAllDirtySettings()` | What the Apply button calls. |
| `RevertAllPendingChanges()` | Pending = Current everywhere. |
| `ResetCategoryToDefault(Category, bApplyImmediately = true)` | |
| `ResetAllCategoriesToDefault(bApplyImmediately = true)` | |
| `HasAnyDirtySettings()` / `IsCategoryDirty(Category)` | |
| `AnyCategoryDirtyStateChangedDelegate` | `(ERageSettingsCategory, bool)` — fires on *transitions*, not on every edit. |
| `CategoryAppliedDelegate` | `(ERageSettingsCategory)` — good hook for reacting to applied settings from gameplay. |

Individual categories also expose their own `DirtyStateChangedDelegate` and typed `SetPendingX()`
functions (`SetPendingMasterVolume`, `SetPendingFieldOfView`, `SetPendingUpscalerSettings`, …) for
code paths that don't go through a row widget. If you write to a Pending field directly by
reflection, call `NotifyPendingChangedExternally(bWasDirtyBefore)` afterwards with the dirty state
captured *before* the write, or the dirty broadcast is missed.

---

## Localization

Hand-authored labels resolve through a string table:

```cpp
#define RAGE_LOC(Key)      FText::FromStringTable(TEXT("/RageSettings/ST_Settings.ST_Settings"), TEXT(Key))
#define RAGE_LOC_Str(Str)  FText::FromStringTable(TEXT("/RageSettings/ST_Settings.ST_Settings"), *Str)
```

`ST_Settings` ships with the plugin. Every user-facing text from RageSettingsUI is localized already.
You will have to handle localization of your own UI/extra settings.

---

## Where things are saved

| Category | Config file |
|---|---|
| Video | `GameUserSettings.ini` (standard `UGameUserSettings` location) |
| Game | `RageGameSettings.ini` |
| Audio | `RageAudioSettings.ini` |
| Input (sensitivity, inversion) | `RageInputSettings.ini` |
| Keybinds | Enhanced Input's own user settings, saved on every remap |

All under `Saved/Config/<Platform>/` at runtime; project defaults go in
`Config/Default<Name>.ini` as usual.

---

## Limitations

- **Input is single-player, primary-slot only.** Local player index 0, one key profile, first mapping
  slot — no split-screen, no per-profile bindings, no secondary keybinds.
- **Keybinds bypass the pending/apply model.** Remaps take effect and save immediately; the Apply
  button and dirty markers never account for them.
- **The row generator handles bool / numeric / enum only.** Structs, arrays, strings and object
  references are skipped — hand-author those rows.
- **The generator never emits a combo row.** Enums become Selection rows unless you override the
  widget class per property. If you absolutely prefer combo rows for such values, you can go `RageSettingsUI::ResolveRowKind`
  and return `EInputRowKind::Combo` instead of `EInputRowKind::Selection`. The advantage of SelectionRows is that they use native FText and
  Combo uses FString parsed from FText.
- **A conflicting rebind unbinds the other action** rather than swapping the two keys.
- Slider clamps in a descriptor bound the widget, not the property. Clamp in a `SetPendingX()` if the
  value must be safe from every path.
- **The subsystem hard-fails as a unit.** If the `GameUserSettings` class isn't
  `URageVideoSettings`, no category initializes. Check the log for
  `URageSettingsSubsystem::Initialize` on startup — it says exactly this.

---

## Third-party software

**This plugin bundles no vendor SDK.** It calls the DLSS, Streamline, FSR, XeSS, XeFG and XeLL APIs
when those plugins are present, and compiles cleanly without any of them — nothing from NVIDIA, AMD
or Intel is redistributed here, and none of their terms attach to this repository.

To ship those features, obtain each plugin from its vendor and enable it in your `.uproject`:

| Plugin | Source |
|---|---|
| DLSS, DLSS Frame Generation (Streamline) | NVIDIA — https://developer.nvidia.com/rtx/dlss |
| FidelityFX Super Resolution (FSR) | AMD — https://gpuopen.com/fidelityfx-super-resolution/ |
| XeSS, XeFG, XeLL | Intel — https://github.com/intel/xess |

**Each is governed by its own license, which you are responsible for reading and complying with.**
They are not MIT and their terms are not implied by this project's.

Unreal Engine itself is licensed separately by Epic Games under the Unreal Engine EULA. This plugin
contains no engine code; it only includes engine headers and links engine modules, as any UE plugin
does.

---

## License

MIT — see [LICENSE](LICENSE). Copyright (c) 2026 Abdallah Boutrif.

Every source file carries an `SPDX-License-Identifier: MIT` tag, so license scanners and compliance
tooling pick it up without parsing prose.
