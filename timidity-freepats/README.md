# FreePats for ModPlug MIDI (desktop share)

Installed / staged only on **win32 / linux / macosx** as:

```text
share/eve/timidity/timidity.cfg
share/eve/timidity/instruments/**/*.pat
```

iOS / Android builds skip this bank (no install, no runtime lookup).

## Setup (once per clone)

```bash
./third-party/timidity-freepats/setup.sh
```

Then reconfigure or rebuild so CMake can stage `instruments/` into the build tree
(`build/<plat>[-debug]/share/eve/timidity`) and SDK install can pick it up.

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
- `instruments/` is gitignored; `timidity.cfg` / `setup.sh` / this README are kept.
- Vendored libmodplug has path-buffer and `WaveHeader.reserved` overflow fixes
  needed to load real `.pat` files safely.
