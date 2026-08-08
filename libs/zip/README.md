# zip

Vendored copy of [kuba--/zip](https://github.com/kuba--/zip), a portable ZIP library
in plain C built on top of [miniz](https://github.com/richgel999/miniz).

* Version: `v0.3.15+miniz.3.1.2` (miniz 3.1.2)
* License: MIT (see `license`)

`ofxLatk` uses this to read and write `.latk` archives. It replaces the bundled
Poco build that used to live in `libs/poco`, which shipped as a 49 MB macOS-only
`.xcframework` and made the addon impossible to build on Linux/ARM (Raspberry Pi).
This library is three source files with no external dependencies, so it compiles
from source on every platform openFrameworks targets.

## Layout

    include/zip.h    public API (does not include miniz.h)
    src/zip.c        implementation
    src/miniz.h      miniz amalgamation, included only by zip.c

## Local patches

Two changes to `src/zip.c`, both marked with an `ofxLatk local patch` comment:

1. `<unistd.h>` is included unconditionally on POSIX. Upstream guards it behind
   `ZIP_HAVE_SYMLINK`, but `ftruncate()` and `unlink()` are called regardless, so
   without the guard defined the build fails under C99+ (clang/gcc treat implicit
   declarations as errors).
2. `MINIZ_NO_ZLIB_COMPATIBLE_NAMES` is defined before `miniz.h` is included, so
   miniz does not declare the zlib-compatible typedefs and macros (`Byte`, `uLong`,
   `Z_OK`, ...) that would collide with the zlib openFrameworks links. `zip.c` only
   uses the `mz_*` names, so nothing else changes.

`ZIP_HAVE_SYMLINK` is intentionally left undefined, which makes `zip_extract()`
write symlink entries as regular files instead of creating symlinks.

## Updating

Copy `src/zip.h` to `include/zip.h` and `src/zip.c` + `src/miniz.h` to `src/` from
a new upstream release, then reapply the two patches above.
