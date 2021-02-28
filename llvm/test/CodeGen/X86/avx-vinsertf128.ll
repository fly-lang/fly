; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -mtriple=x86_64-unknown-unknown -mattr=avx | FileCheck %s

define <8 x float> @A(<8 x float> %a) nounwind uwtable readnone ssp {
; CHECK-LABEL: A:
; CHECK:       # %bb.0:
; CHECK-NEXT:    vinsertf128 $1, %xmm0, %ymm0, %ymm0
; CHECK-NEXT:    retq
  %shuffle = shufflevector <8 x float> %a, <8 x float> undef, <8 x i32> <i32 8, i32 8, i32 8, i32 8, i32 0, i32 1, i32 2, i32 3>
  ret <8 x float> %shuffle
}

define <4 x double> @B(<4 x double> %a) nounwind uwtable readnone ssp {
; CHECK-LABEL: B:
; CHECK:       # %bb.0:
; CHECK-NEXT:    vinsertf128 $1, %xmm0, %ymm0, %ymm0
; CHECK-NEXT:    retq
  %shuffle = shufflevector <4 x double> %a, <4 x double> undef, <4 x i32> <i32 4, i32 4, i32 0, i32 1>
  ret <4 x double> %shuffle
}

declare <2 x double> @llvm.x86.sse2.min.pd(<2 x double>, <2 x double>) nounwind readnone
declare <2 x double> @llvm.x86.sse2.min.sd(<2 x double>, <2 x double>) nounwind readnone

define void @insert_crash() nounwind {
; CHECK-LABEL: insert_crash:
; CHECK:       # %bb.0: # %allocas
; CHECK-NEXT:    vxorpd %xmm0, %xmm0, %xmm0
; CHECK-NEXT:    vminpd %xmm0, %xmm0, %xmm0
; CHECK-NEXT:    vminsd %xmm0, %xmm0, %xmm0
; CHECK-NEXT:    vcvtsd2ss %xmm0, %xmm0, %xmm0
; CHECK-NEXT:    vpermilps {{.*#+}} xmm0 = xmm0[0,1,2,0]
; CHECK-NEXT:    vmovups %xmm0, (%rax)
; CHECK-NEXT:    retq
allocas:
  %v1.i.i451 = shufflevector <4 x double> zeroinitializer, <4 x double> undef, <4 x i32> <i32 2, i32 3, i32 undef, i32 undef>
  %ret_0a.i.i.i452 = shufflevector <4 x double> %v1.i.i451, <4 x double> undef, <2 x i32> <i32 0, i32 1>
  %vret_0.i.i.i454 = tail call <2 x double> @llvm.x86.sse2.min.pd(<2 x double> %ret_0a.i.i.i452, <2 x double> undef) nounwind
  %ret_val.i.i.i463 = tail call <2 x double> @llvm.x86.sse2.min.sd(<2 x double> %vret_0.i.i.i454, <2 x double> undef) nounwind
  %ret.i1.i.i464 = extractelement <2 x double> %ret_val.i.i.i463, i32 0
  %double2float = fptrunc double %ret.i1.i.i464 to float
  %smearinsert50 = insertelement <4 x float> undef, float %double2float, i32 3
  %blendAsInt.i503 = bitcast <4 x float> %smearinsert50 to <4 x i32>
  store <4 x i32> %blendAsInt.i503, <4 x i32>* undef, align 4
  ret void
}

;; DAG Combine must remove useless vinsertf128 instructions

define <4 x i32> @DAGCombineA(<4 x i32> %v1) nounwind readonly {
; CHECK-LABEL: DAGCombineA:
; CHECK:       # %bb.0:
; CHECK-NEXT:    retq
  %t1 = shufflevector <4 x i32> %v1, <4 x i32> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %t2 = shufflevector <8 x i32> %t1, <8 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  ret <4 x i32> %t2
}

define <8 x i32> @DAGCombineB(<8 x i32> %v1, <8 x i32> %v2) nounwind readonly {
; CHECK-LABEL: DAGCombineB:
; CHECK:       # %bb.0:
; CHECK-NEXT:    vextractf128 $1, %ymm1, %xmm2
; CHECK-NEXT:    vextractf128 $1, %ymm0, %xmm3
; CHECK-NEXT:    vpaddd %xmm3, %xmm2, %xmm2
; CHECK-NEXT:    vpaddd %xmm2, %xmm3, %xmm2
; CHECK-NEXT:    vpaddd %xmm0, %xmm1, %xmm1
; CHECK-NEXT:    vpaddd %xmm1, %xmm0, %xmm0
; CHECK-NEXT:    vinsertf128 $1, %xmm2, %ymm0, %ymm0
; CHECK-NEXT:    retq
  %t1 = add <8 x i32> %v1, %v2
  %t2 = add <8 x i32> %t1, %v1
  ret <8 x i32> %t2
}

define <4 x double> @insert_undef_pd(<4 x double> %a0, <2 x double> %a1) {
; CHECK-LABEL: insert_undef_pd:
; CHECK:       # %bb.0:
; CHECK-NEXT:    vmovaps %xmm1, %xmm0
; CHECK-NEXT:    retq
%res = call <4 x double> @llvm.x86.avx.vinsertf128.pd.256(<4 x double> undef, <2 x double> %a1, i8 0)
ret <4 x double> %res
}
declare <4 x double> @llvm.x86.avx.vinsertf128.pd.256(<4 x double>, <2 x double>, i8) nounwind readnone

define <8 x float> @insert_undef_ps(<8 x float> %a0, <4 x float> %a1) {
; CHECK-LABEL: insert_undef_ps:
; CHECK:       # %bb.0:
; CHECK-NEXT:    vmovaps %xmm1, %xmm0
; CHECK-NEXT:    retq
%res = call <8 x float> @llvm.x86.avx.vinsertf128.ps.256(<8 x float> undef, <4 x float> %a1, i8 0)
ret <8 x float> %res
}
declare <8 x float> @llvm.x86.avx.vinsertf128.ps.256(<8 x float>, <4 x float>, i8) nounwind readnone

define <8 x i32> @insert_undef_si(<8 x i32> %a0, <4 x i32> %a1) {
; CHECK-LABEL: insert_undef_si:
; CHECK:       # %bb.0:
; CHECK-NEXT:    vmovaps %xmm1, %xmm0
; CHECK-NEXT:    retq
%res = call <8 x i32> @llvm.x86.avx.vinsertf128.si.256(<8 x i32> undef, <4 x i32> %a1, i8 0)
ret <8 x i32> %res
}
declare <8 x i32> @llvm.x86.avx.vinsertf128.si.256(<8 x i32>, <4 x i32>, i8) nounwind readnone

; rdar://10643481
define <8 x float> @vinsertf128_combine(float* nocapture %f) nounwind uwtable readonly ssp {
; CHECK-LABEL: vinsertf128_combine:
; CHECK:       # %bb.0:
; CHECK-NEXT:    vinsertf128 $1, 16(%rdi), %ymm0, %ymm0
; CHECK-NEXT:    retq
  %add.ptr = getelementptr inbounds float, float* %f, i64 4
  %t0 = bitcast float* %add.ptr to <4 x float>*
  %t1 = load <4 x float>, <4 x float>* %t0, align 16
  %t2 = tail call <8 x float> @llvm.x86.avx.vinsertf128.ps.256(<8 x float> undef, <4 x float> %t1, i8 1)
  ret <8 x float> %t2
}

; rdar://11076953
define <8 x float> @vinsertf128_ucombine(float* nocapture %f) nounwind uwtable readonly ssp {
; CHECK-LABEL: vinsertf128_ucombine:
; CHECK:       # %bb.0:
; CHECK-NEXT:    vinsertf128 $1, 16(%rdi), %ymm0, %ymm0
; CHECK-NEXT:    retq
  %add.ptr = getelementptr inbounds float, float* %f, i64 4
  %t0 = bitcast float* %add.ptr to <4 x float>*
  %t1 = load <4 x float>, <4 x float>* %t0, align 8
  %t2 = tail call <8 x float> @llvm.x86.avx.vinsertf128.ps.256(<8 x float> undef, <4 x float> %t1, i8 1)
  ret <8 x float> %t2
}

