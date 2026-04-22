# xv6-os toolchain sub-repo

Cross binutils + GCC + xv6-patched musl libc, built by a single
two-phase bash script lifted verbatim from upstream xv6.

## Layout

```
toolchain/
├── scripts/
│   └── build_gcc_toolchain.sh   monolithic build driver (Phase 1 + 2)
└── musl-xv6/                    xv6 syscall overlay applied to upstream musl
    ├── apply_xv6_overlay.sh
    ├── arch/
    │   ├── riscv64/             syscall.h.in, kstat.h, clone.s, ...
    │   └── x86_64/
    ├── compat/                  small compat shims (mmap helpers etc)
    ├── programs/                test programs against the overlaid musl
    ├── build_musl.sh            standalone musl-only build (used by CI)
    └── build_musl_x86_64.sh
```

## Phases

| phase | what                                  | output prefix                                       |
|-------|---------------------------------------|-----------------------------------------------------|
| 1     | binutils + gcc (static) + musl static | `${PREFIX}/${arch}/phase1/bin/${triple}-*`          |
| 2     | rebuild gcc + musl with shared libs   | `${PREFIX}/${arch}/phase2/bin/${triple}-*`          |

The triple is `${arch}-xv6-linux-musl` (e.g. `riscv64-xv6-linux-musl`).
The "linux" OS field is critical — it makes binutils enable ELF
shared-object support without needing a custom target patch.

## Standalone build

```sh
./scripts/build_gcc_toolchain.sh \
    --arch=riscv64 \
    --prefix=$PWD/install \
    --jobs=$(nproc)

# Phase 1 only (kernel + xv6-native userland just need this):
./scripts/build_gcc_toolchain.sh --arch=riscv64 --prefix=... --phase=1
```

After Phase 2 completes, prepend `${PREFIX}/riscv64/phase2/bin` to
`PATH` and you have a working `riscv64-xv6-linux-musl-gcc`.

### Host prerequisites

```
build-essential gcc g++ make texinfo bison flex
libgmp-dev libmpfr-dev libmpc-dev zlib1g-dev libexpat-dev
wget tar gawk
```

## Versions (default)

* GCC 14.2.0
* binutils 2.43
* musl 1.2.5

Override with `--gcc-version=`, `--binutils-version=`, `--musl-version=`.

## Umbrella usage

`xv6-os/cmake/BuildToolchain.cmake` invokes the script twice (once
per phase) inside `ExternalProject`s:

* `tc-phase1` → produces Phase 1 toolchain. Aliased as `tc-binutils`,
  `tc-gcc-stage1`, `tc-musl` so the kernel/user/ports sub-repos can
  depend on the minimum stage they need.
* `tc-phase2` → produces Phase 2 (depends on `tc-phase1`).

`XV6_TOOLCHAIN_BIN` in the umbrella resolves to the Phase 2 bin dir;
ports needing dynamic linking pick that up automatically.
