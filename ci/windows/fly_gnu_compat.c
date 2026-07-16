/* fly_gnu_compat.c - CRT-glue for the Windows GNU/UCRT (gnullvm) toolchain.
 *
 * The stage0 reference compiler (and, until the strada-2 backend fix, the
 * self-host) emit MSVC-style codegen even for the x86_64-w64-windows-gnu triple:
 *   * a `_fltused` marker whenever floating point is used, and
 *   * `call __chkstk` (the MSVC stack-probe name) for frames > 1 page.
 * The mingw/UCRT sysroot provides neither - it has the GNU `___chkstk_ms`
 * instead. This tiny object bridges the gap:
 *   * defines `_fltused` (value is irrelevant; only its presence matters), and
 *   * defines `__chkstk` as a tail-jump to `___chkstk_ms`. The two share the
 *     exact caller contract on x86_64 (RAX = size in, RSP untouched, caller does
 *     `sub rsp, rax`); ___chkstk_ms merely clobbers fewer registers, so the
 *     alias is ABI-safe.
 *
 * Regenerate (from an llvm-mingw toolchain):
 *   clang --target=x86_64-w64-windows-gnu -c -Os fly_gnu_compat.c -o fly_gnu_compat.o
 */
int _fltused = 0x9875;

__asm__(
    ".global __chkstk\n"
    "__chkstk:\n"
    "    jmp ___chkstk_ms\n"
);
