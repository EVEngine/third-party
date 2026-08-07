# FreePats for ModPlug MIDI (desktop share)

Installed / staged only on **win32 / linux / macosx** as:

```text
share/eve/timidity/timidity.cfg
share/eve/timidity/instruments/**/*.pat
```

iOS / Android builds skip this bank (no install, no runtime lookup).

## Contents

`instruments/` (FreePats GUS patches, ~32MB) is **tracked in this repo**. After
cloning `third-party`, reconfigure or rebuild so CMake stages the bank into
`build/<plat>[-debug]/share/eve/timidity` (and SDK install can pick it up).

Optional: re-download / regenerate from the Debian package with:

```bash
./timidity-freepats/setup.sh
```

## Runtime

`eve::sound::Sound` sets `MMPAT_PATH_TO_CFG` to the staged/installed share path
when the env var is unset. Override or trim freely:

```bash
# use a trimmed bank
export MMPAT_PATH_TO_CFG=/path/to/my-small-timidity
```

## Trim the bank

FreePats is ~32MB. Keep only the `.pat` files your songs need and edit
`timidity.cfg` accordingly (melodic bank before `drumset`; paths without
`.pat` suffix, relative to `instruments/`).

## Notes

- FreePats does not cover every GM program; ModPlug substitutes nearest patches.
- Vendored libmodplug has path-buffer and `WaveHeader.reserved` overflow fixes
  needed to load real `.pat` files safely.
