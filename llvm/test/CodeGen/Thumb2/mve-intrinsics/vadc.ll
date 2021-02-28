; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc -mtriple=thumbv8.1m.main -mattr=+mve.fp -verify-machineinstrs -o - %s | FileCheck %s

define arm_aapcs_vfpcc <4 x i32> @test_vadciq_s32(<4 x i32> %a, <4 x i32> %b, i32* %carry_out) {
; CHECK-LABEL: test_vadciq_s32:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vadci.i32 q0, q0, q1
; CHECK-NEXT:    vmrs r1, fpscr_nzcvqc
; CHECK-NEXT:    ubfx r1, r1, #29, #1
; CHECK-NEXT:    str r1, [r0]
; CHECK-NEXT:    bx lr
entry:
  %0 = tail call { <4 x i32>, i32 } @llvm.arm.mve.vadc.v4i32(<4 x i32> %a, <4 x i32> %b, i32 0)
  %1 = extractvalue { <4 x i32>, i32 } %0, 1
  %2 = lshr i32 %1, 29
  %3 = and i32 %2, 1
  store i32 %3, i32* %carry_out, align 4
  %4 = extractvalue { <4 x i32>, i32 } %0, 0
  ret <4 x i32> %4
}

declare { <4 x i32>, i32 } @llvm.arm.mve.vadc.v4i32(<4 x i32>, <4 x i32>, i32)

define arm_aapcs_vfpcc <4 x i32> @test_vadcq_u32(<4 x i32> %a, <4 x i32> %b, i32* %carry) {
; CHECK-LABEL: test_vadcq_u32:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    ldr r1, [r0]
; CHECK-NEXT:    lsls r1, r1, #29
; CHECK-NEXT:    vmsr fpscr_nzcvqc, r1
; CHECK-NEXT:    vadc.i32 q0, q0, q1
; CHECK-NEXT:    vmrs r1, fpscr_nzcvqc
; CHECK-NEXT:    ubfx r1, r1, #29, #1
; CHECK-NEXT:    str r1, [r0]
; CHECK-NEXT:    bx lr
entry:
  %0 = load i32, i32* %carry, align 4
  %1 = shl i32 %0, 29
  %2 = tail call { <4 x i32>, i32 } @llvm.arm.mve.vadc.v4i32(<4 x i32> %a, <4 x i32> %b, i32 %1)
  %3 = extractvalue { <4 x i32>, i32 } %2, 1
  %4 = lshr i32 %3, 29
  %5 = and i32 %4, 1
  store i32 %5, i32* %carry, align 4
  %6 = extractvalue { <4 x i32>, i32 } %2, 0
  ret <4 x i32> %6
}

define arm_aapcs_vfpcc <4 x i32> @test_vadciq_m_u32(<4 x i32> %inactive, <4 x i32> %a, <4 x i32> %b, i32* %carry_out, i16 zeroext %p) {
; CHECK-LABEL: test_vadciq_m_u32:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vmsr p0, r1
; CHECK-NEXT:    vpst
; CHECK-NEXT:    vadcit.i32 q0, q1, q2
; CHECK-NEXT:    vmrs r1, fpscr_nzcvqc
; CHECK-NEXT:    ubfx r1, r1, #29, #1
; CHECK-NEXT:    str r1, [r0]
; CHECK-NEXT:    bx lr
entry:
  %0 = zext i16 %p to i32
  %1 = tail call <4 x i1> @llvm.arm.mve.pred.i2v.v4i1(i32 %0)
  %2 = tail call { <4 x i32>, i32 } @llvm.arm.mve.vadc.predicated.v4i32.v4i1(<4 x i32> %inactive, <4 x i32> %a, <4 x i32> %b, i32 0, <4 x i1> %1)
  %3 = extractvalue { <4 x i32>, i32 } %2, 1
  %4 = lshr i32 %3, 29
  %5 = and i32 %4, 1
  store i32 %5, i32* %carry_out, align 4
  %6 = extractvalue { <4 x i32>, i32 } %2, 0
  ret <4 x i32> %6
}

declare <4 x i1> @llvm.arm.mve.pred.i2v.v4i1(i32)

declare { <4 x i32>, i32 } @llvm.arm.mve.vadc.predicated.v4i32.v4i1(<4 x i32>, <4 x i32>, <4 x i32>, i32, <4 x i1>)

define arm_aapcs_vfpcc <4 x i32> @test_vadcq_m_s32(<4 x i32> %inactive, <4 x i32> %a, <4 x i32> %b, i32* %carry, i16 zeroext %p) {
; CHECK-LABEL: test_vadcq_m_s32:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    ldr r2, [r0]
; CHECK-NEXT:    vmsr p0, r1
; CHECK-NEXT:    lsls r1, r2, #29
; CHECK-NEXT:    vmsr fpscr_nzcvqc, r1
; CHECK-NEXT:    vpst
; CHECK-NEXT:    vadct.i32 q0, q1, q2
; CHECK-NEXT:    vmrs r1, fpscr_nzcvqc
; CHECK-NEXT:    ubfx r1, r1, #29, #1
; CHECK-NEXT:    str r1, [r0]
; CHECK-NEXT:    bx lr
entry:
  %0 = load i32, i32* %carry, align 4
  %1 = shl i32 %0, 29
  %2 = zext i16 %p to i32
  %3 = tail call <4 x i1> @llvm.arm.mve.pred.i2v.v4i1(i32 %2)
  %4 = tail call { <4 x i32>, i32 } @llvm.arm.mve.vadc.predicated.v4i32.v4i1(<4 x i32> %inactive, <4 x i32> %a, <4 x i32> %b, i32 %1, <4 x i1> %3)
  %5 = extractvalue { <4 x i32>, i32 } %4, 1
  %6 = lshr i32 %5, 29
  %7 = and i32 %6, 1
  store i32 %7, i32* %carry, align 4
  %8 = extractvalue { <4 x i32>, i32 } %4, 0
  ret <4 x i32> %8
}
