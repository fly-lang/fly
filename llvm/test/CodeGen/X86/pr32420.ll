; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s | FileCheck %s

target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

@a = common local_unnamed_addr global i16 0, align 4
@b = common local_unnamed_addr global i16 0, align 4

define i32 @PR32420() {
; CHECK-LABEL: PR32420:
; CHECK:       ## %bb.0:
; CHECK-NEXT:    movq _a@{{.*}}(%rip), %rcx
; CHECK-NEXT:    movzwl (%rcx), %eax
; CHECK-NEXT:    movl %eax, %edx
; CHECK-NEXT:    shll $12, %edx
; CHECK-NEXT:    movswl %dx, %edx
; CHECK-NEXT:    shrl $12, %edx
; CHECK-NEXT:    movq _b@{{.*}}(%rip), %rsi
; CHECK-NEXT:    orw (%rsi), %dx
; CHECK-NEXT:    movl (%rcx), %ecx
; CHECK-NEXT:    shll $12, %ecx
; CHECK-NEXT:    movswl %cx, %ecx
; CHECK-NEXT:    shrl $12, %ecx
; CHECK-NEXT:    andl %edx, %ecx
; CHECK-NEXT:    movw %cx, (%rsi)
; CHECK-NEXT:    retq
  %load2 = load i16, i16* @a, align 4
  %shl3 = shl i16 %load2, 12
  %ashr4 = ashr i16 %shl3, 12
  %t2 = load volatile i16, i16* @b, align 4
  %conv8 = or i16 %t2, %ashr4
  %load9 = load i16, i16* @a, align 4
  %shl10 = shl i16 %load9, 12
  %ashr11 = ashr i16 %shl10, 12
  %and = and i16 %conv8, %ashr11
  store i16 %and, i16* @b, align 4
  %cast1629 = zext i16 %load2 to i32
  ret i32 %cast1629
}
