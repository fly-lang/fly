; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -mtriple=i686-unknown -mattr=+sse2 | FileCheck %s --check-prefix=X32-SSE
; RUN: llc < %s -mtriple=i686-unknown -mattr=+avx | FileCheck %s --check-prefix=X32-AVX
; RUN: llc < %s -mtriple=x86_64-unknown -mattr=+sse2 | FileCheck %s --check-prefix=X64-SSE
; RUN: llc < %s -mtriple=x86_64-unknown -mattr=+avx | FileCheck %s --check-prefix=X64-AVX

;PR29078

define <2 x double> @mask_sitofp_2i64_2f64(<2 x i64> %a) nounwind {
; X32-SSE-LABEL: mask_sitofp_2i64_2f64:
; X32-SSE:       # %bb.0:
; X32-SSE-NEXT:    pshufd {{.*#+}} xmm0 = xmm0[0,2,2,3]
; X32-SSE-NEXT:    pand {{\.LCPI.*}}, %xmm0
; X32-SSE-NEXT:    cvtdq2pd %xmm0, %xmm0
; X32-SSE-NEXT:    retl
;
; X32-AVX-LABEL: mask_sitofp_2i64_2f64:
; X32-AVX:       # %bb.0:
; X32-AVX-NEXT:    vpshufb {{.*#+}} xmm0 = xmm0[0],zero,zero,zero,xmm0[8,9],zero,zero,xmm0[u,u,u,u,u,u,u,u]
; X32-AVX-NEXT:    vcvtdq2pd %xmm0, %xmm0
; X32-AVX-NEXT:    retl
;
; X64-SSE-LABEL: mask_sitofp_2i64_2f64:
; X64-SSE:       # %bb.0:
; X64-SSE-NEXT:    pshufd {{.*#+}} xmm0 = xmm0[0,2,2,3]
; X64-SSE-NEXT:    pand {{.*}}(%rip), %xmm0
; X64-SSE-NEXT:    cvtdq2pd %xmm0, %xmm0
; X64-SSE-NEXT:    retq
;
; X64-AVX-LABEL: mask_sitofp_2i64_2f64:
; X64-AVX:       # %bb.0:
; X64-AVX-NEXT:    vpshufb {{.*#+}} xmm0 = xmm0[0],zero,zero,zero,xmm0[8,9],zero,zero,xmm0[u,u,u,u,u,u,u,u]
; X64-AVX-NEXT:    vcvtdq2pd %xmm0, %xmm0
; X64-AVX-NEXT:    retq
  %and = and <2 x i64> %a, <i64 255, i64 65535>
  %cvt = sitofp <2 x i64> %and to <2 x double>
  ret <2 x double> %cvt
}

define <2 x double> @mask_uitofp_2i64_2f64(<2 x i64> %a) nounwind {
; X32-SSE-LABEL: mask_uitofp_2i64_2f64:
; X32-SSE:       # %bb.0:
; X32-SSE-NEXT:    pshufd {{.*#+}} xmm0 = xmm0[0,2,2,3]
; X32-SSE-NEXT:    pand {{\.LCPI.*}}, %xmm0
; X32-SSE-NEXT:    cvtdq2pd %xmm0, %xmm0
; X32-SSE-NEXT:    retl
;
; X32-AVX-LABEL: mask_uitofp_2i64_2f64:
; X32-AVX:       # %bb.0:
; X32-AVX-NEXT:    vpshufb {{.*#+}} xmm0 = xmm0[0],zero,zero,zero,xmm0[8,9],zero,zero,xmm0[u,u,u,u,u,u,u,u]
; X32-AVX-NEXT:    vcvtdq2pd %xmm0, %xmm0
; X32-AVX-NEXT:    retl
;
; X64-SSE-LABEL: mask_uitofp_2i64_2f64:
; X64-SSE:       # %bb.0:
; X64-SSE-NEXT:    pshufd {{.*#+}} xmm0 = xmm0[0,2,2,3]
; X64-SSE-NEXT:    pand {{.*}}(%rip), %xmm0
; X64-SSE-NEXT:    cvtdq2pd %xmm0, %xmm0
; X64-SSE-NEXT:    retq
;
; X64-AVX-LABEL: mask_uitofp_2i64_2f64:
; X64-AVX:       # %bb.0:
; X64-AVX-NEXT:    vpshufb {{.*#+}} xmm0 = xmm0[0],zero,zero,zero,xmm0[8,9],zero,zero,xmm0[u,u,u,u,u,u,u,u]
; X64-AVX-NEXT:    vcvtdq2pd %xmm0, %xmm0
; X64-AVX-NEXT:    retq
  %and = and <2 x i64> %a, <i64 255, i64 65535>
  %cvt = uitofp <2 x i64> %and to <2 x double>
  ret <2 x double> %cvt
}

define <4 x float> @mask_sitofp_4i64_4f32(<4 x i64> %a) nounwind {
; X32-SSE-LABEL: mask_sitofp_4i64_4f32:
; X32-SSE:       # %bb.0:
; X32-SSE-NEXT:    shufps {{.*#+}} xmm0 = xmm0[0,2],xmm1[0,2]
; X32-SSE-NEXT:    andps {{\.LCPI.*}}, %xmm0
; X32-SSE-NEXT:    cvtdq2ps %xmm0, %xmm0
; X32-SSE-NEXT:    retl
;
; X32-AVX-LABEL: mask_sitofp_4i64_4f32:
; X32-AVX:       # %bb.0:
; X32-AVX-NEXT:    vextractf128 $1, %ymm0, %xmm1
; X32-AVX-NEXT:    vshufps {{.*#+}} xmm0 = xmm0[0,2],xmm1[0,2]
; X32-AVX-NEXT:    vandps {{\.LCPI.*}}, %xmm0, %xmm0
; X32-AVX-NEXT:    vcvtdq2ps %xmm0, %xmm0
; X32-AVX-NEXT:    vzeroupper
; X32-AVX-NEXT:    retl
;
; X64-SSE-LABEL: mask_sitofp_4i64_4f32:
; X64-SSE:       # %bb.0:
; X64-SSE-NEXT:    shufps {{.*#+}} xmm0 = xmm0[0,2],xmm1[0,2]
; X64-SSE-NEXT:    andps {{.*}}(%rip), %xmm0
; X64-SSE-NEXT:    cvtdq2ps %xmm0, %xmm0
; X64-SSE-NEXT:    retq
;
; X64-AVX-LABEL: mask_sitofp_4i64_4f32:
; X64-AVX:       # %bb.0:
; X64-AVX-NEXT:    vextractf128 $1, %ymm0, %xmm1
; X64-AVX-NEXT:    vshufps {{.*#+}} xmm0 = xmm0[0,2],xmm1[0,2]
; X64-AVX-NEXT:    vandps {{.*}}(%rip), %xmm0, %xmm0
; X64-AVX-NEXT:    vcvtdq2ps %xmm0, %xmm0
; X64-AVX-NEXT:    vzeroupper
; X64-AVX-NEXT:    retq
  %and = and <4 x i64> %a, <i64 127, i64 255, i64 4095, i64 65535>
  %cvt = sitofp <4 x i64> %and to <4 x float>
  ret <4 x float> %cvt
}

define <4 x float> @mask_uitofp_4i64_4f32(<4 x i64> %a) nounwind {
; X32-SSE-LABEL: mask_uitofp_4i64_4f32:
; X32-SSE:       # %bb.0:
; X32-SSE-NEXT:    shufps {{.*#+}} xmm0 = xmm0[0,2],xmm1[0,2]
; X32-SSE-NEXT:    andps {{\.LCPI.*}}, %xmm0
; X32-SSE-NEXT:    cvtdq2ps %xmm0, %xmm0
; X32-SSE-NEXT:    retl
;
; X32-AVX-LABEL: mask_uitofp_4i64_4f32:
; X32-AVX:       # %bb.0:
; X32-AVX-NEXT:    vextractf128 $1, %ymm0, %xmm1
; X32-AVX-NEXT:    vshufps {{.*#+}} xmm0 = xmm0[0,2],xmm1[0,2]
; X32-AVX-NEXT:    vandps {{\.LCPI.*}}, %xmm0, %xmm0
; X32-AVX-NEXT:    vcvtdq2ps %xmm0, %xmm0
; X32-AVX-NEXT:    vzeroupper
; X32-AVX-NEXT:    retl
;
; X64-SSE-LABEL: mask_uitofp_4i64_4f32:
; X64-SSE:       # %bb.0:
; X64-SSE-NEXT:    shufps {{.*#+}} xmm0 = xmm0[0,2],xmm1[0,2]
; X64-SSE-NEXT:    andps {{.*}}(%rip), %xmm0
; X64-SSE-NEXT:    cvtdq2ps %xmm0, %xmm0
; X64-SSE-NEXT:    retq
;
; X64-AVX-LABEL: mask_uitofp_4i64_4f32:
; X64-AVX:       # %bb.0:
; X64-AVX-NEXT:    vextractf128 $1, %ymm0, %xmm1
; X64-AVX-NEXT:    vshufps {{.*#+}} xmm0 = xmm0[0,2],xmm1[0,2]
; X64-AVX-NEXT:    vandps {{.*}}(%rip), %xmm0, %xmm0
; X64-AVX-NEXT:    vcvtdq2ps %xmm0, %xmm0
; X64-AVX-NEXT:    vzeroupper
; X64-AVX-NEXT:    retq
  %and = and <4 x i64> %a, <i64 127, i64 255, i64 4095, i64 65535>
  %cvt = uitofp <4 x i64> %and to <4 x float>
  ret <4 x float> %cvt
}

define <2 x double> @clamp_sitofp_2i64_2f64(<2 x i64> %a) nounwind {
; X32-SSE-LABEL: clamp_sitofp_2i64_2f64:
; X32-SSE:       # %bb.0:
; X32-SSE-NEXT:    movdqa {{.*#+}} xmm1 = [2147483648,0,2147483648,0]
; X32-SSE-NEXT:    movdqa %xmm0, %xmm2
; X32-SSE-NEXT:    pxor %xmm1, %xmm2
; X32-SSE-NEXT:    movdqa {{.*#+}} xmm3 = [2147483393,4294967295,2147483393,4294967295]
; X32-SSE-NEXT:    movdqa %xmm2, %xmm4
; X32-SSE-NEXT:    pcmpgtd %xmm3, %xmm4
; X32-SSE-NEXT:    pshufd {{.*#+}} xmm5 = xmm4[0,0,2,2]
; X32-SSE-NEXT:    pcmpeqd %xmm3, %xmm2
; X32-SSE-NEXT:    pshufd {{.*#+}} xmm2 = xmm2[1,1,3,3]
; X32-SSE-NEXT:    pand %xmm5, %xmm2
; X32-SSE-NEXT:    pshufd {{.*#+}} xmm3 = xmm4[1,1,3,3]
; X32-SSE-NEXT:    por %xmm2, %xmm3
; X32-SSE-NEXT:    pand %xmm3, %xmm0
; X32-SSE-NEXT:    pandn {{\.LCPI.*}}, %xmm3
; X32-SSE-NEXT:    por %xmm0, %xmm3
; X32-SSE-NEXT:    pxor %xmm3, %xmm1
; X32-SSE-NEXT:    movdqa {{.*#+}} xmm0 = [2147483903,0,2147483903,0]
; X32-SSE-NEXT:    movdqa %xmm0, %xmm2
; X32-SSE-NEXT:    pcmpgtd %xmm1, %xmm2
; X32-SSE-NEXT:    pshufd {{.*#+}} xmm4 = xmm2[0,0,2,2]
; X32-SSE-NEXT:    pcmpeqd %xmm0, %xmm1
; X32-SSE-NEXT:    pshufd {{.*#+}} xmm0 = xmm1[1,1,3,3]
; X32-SSE-NEXT:    pand %xmm4, %xmm0
; X32-SSE-NEXT:    pshufd {{.*#+}} xmm1 = xmm2[1,1,3,3]
; X32-SSE-NEXT:    por %xmm0, %xmm1
; X32-SSE-NEXT:    pand %xmm1, %xmm3
; X32-SSE-NEXT:    pandn {{\.LCPI.*}}, %xmm1
; X32-SSE-NEXT:    por %xmm3, %xmm1
; X32-SSE-NEXT:    pshufd {{.*#+}} xmm0 = xmm1[0,2,2,3]
; X32-SSE-NEXT:    cvtdq2pd %xmm0, %xmm0
; X32-SSE-NEXT:    retl
;
; X32-AVX-LABEL: clamp_sitofp_2i64_2f64:
; X32-AVX:       # %bb.0:
; X32-AVX-NEXT:    vmovddup {{.*#+}} xmm1 = [NaN,NaN]
; X32-AVX-NEXT:    # xmm1 = mem[0,0]
; X32-AVX-NEXT:    vpcmpgtq %xmm1, %xmm0, %xmm2
; X32-AVX-NEXT:    vblendvpd %xmm2, %xmm0, %xmm1, %xmm0
; X32-AVX-NEXT:    vmovddup {{.*#+}} xmm1 = [1.2598673968951787E-321,1.2598673968951787E-321]
; X32-AVX-NEXT:    # xmm1 = mem[0,0]
; X32-AVX-NEXT:    vpcmpgtq %xmm0, %xmm1, %xmm2
; X32-AVX-NEXT:    vblendvpd %xmm2, %xmm0, %xmm1, %xmm0
; X32-AVX-NEXT:    vpermilps {{.*#+}} xmm0 = xmm0[0,2,2,3]
; X32-AVX-NEXT:    vcvtdq2pd %xmm0, %xmm0
; X32-AVX-NEXT:    retl
;
; X64-SSE-LABEL: clamp_sitofp_2i64_2f64:
; X64-SSE:       # %bb.0:
; X64-SSE-NEXT:    movdqa {{.*#+}} xmm1 = [2147483648,2147483648]
; X64-SSE-NEXT:    movdqa %xmm0, %xmm2
; X64-SSE-NEXT:    pxor %xmm1, %xmm2
; X64-SSE-NEXT:    movdqa {{.*#+}} xmm3 = [18446744071562067713,18446744071562067713]
; X64-SSE-NEXT:    movdqa %xmm2, %xmm4
; X64-SSE-NEXT:    pcmpgtd %xmm3, %xmm4
; X64-SSE-NEXT:    pshufd {{.*#+}} xmm5 = xmm4[0,0,2,2]
; X64-SSE-NEXT:    pcmpeqd %xmm3, %xmm2
; X64-SSE-NEXT:    pshufd {{.*#+}} xmm2 = xmm2[1,1,3,3]
; X64-SSE-NEXT:    pand %xmm5, %xmm2
; X64-SSE-NEXT:    pshufd {{.*#+}} xmm3 = xmm4[1,1,3,3]
; X64-SSE-NEXT:    por %xmm2, %xmm3
; X64-SSE-NEXT:    pand %xmm3, %xmm0
; X64-SSE-NEXT:    pandn {{.*}}(%rip), %xmm3
; X64-SSE-NEXT:    por %xmm0, %xmm3
; X64-SSE-NEXT:    pxor %xmm3, %xmm1
; X64-SSE-NEXT:    movdqa {{.*#+}} xmm0 = [2147483903,2147483903]
; X64-SSE-NEXT:    movdqa %xmm0, %xmm2
; X64-SSE-NEXT:    pcmpgtd %xmm1, %xmm2
; X64-SSE-NEXT:    pshufd {{.*#+}} xmm4 = xmm2[0,0,2,2]
; X64-SSE-NEXT:    pcmpeqd %xmm0, %xmm1
; X64-SSE-NEXT:    pshufd {{.*#+}} xmm0 = xmm1[1,1,3,3]
; X64-SSE-NEXT:    pand %xmm4, %xmm0
; X64-SSE-NEXT:    pshufd {{.*#+}} xmm1 = xmm2[1,1,3,3]
; X64-SSE-NEXT:    por %xmm0, %xmm1
; X64-SSE-NEXT:    pand %xmm1, %xmm3
; X64-SSE-NEXT:    pandn {{.*}}(%rip), %xmm1
; X64-SSE-NEXT:    por %xmm3, %xmm1
; X64-SSE-NEXT:    pshufd {{.*#+}} xmm0 = xmm1[0,2,2,3]
; X64-SSE-NEXT:    cvtdq2pd %xmm0, %xmm0
; X64-SSE-NEXT:    retq
;
; X64-AVX-LABEL: clamp_sitofp_2i64_2f64:
; X64-AVX:       # %bb.0:
; X64-AVX-NEXT:    vmovdqa {{.*#+}} xmm1 = [18446744073709551361,18446744073709551361]
; X64-AVX-NEXT:    vpcmpgtq %xmm1, %xmm0, %xmm2
; X64-AVX-NEXT:    vblendvpd %xmm2, %xmm0, %xmm1, %xmm0
; X64-AVX-NEXT:    vmovdqa {{.*#+}} xmm1 = [255,255]
; X64-AVX-NEXT:    vpcmpgtq %xmm0, %xmm1, %xmm2
; X64-AVX-NEXT:    vblendvpd %xmm2, %xmm0, %xmm1, %xmm0
; X64-AVX-NEXT:    vpermilps {{.*#+}} xmm0 = xmm0[0,2,2,3]
; X64-AVX-NEXT:    vcvtdq2pd %xmm0, %xmm0
; X64-AVX-NEXT:    retq
  %clo = icmp slt <2 x i64> %a, <i64 -255, i64 -255>
  %lo = select <2 x i1> %clo, <2 x i64> <i64 -255, i64 -255>, <2 x i64> %a
  %chi = icmp sgt <2 x i64> %lo, <i64 255, i64 255>
  %hi = select <2 x i1> %chi, <2 x i64> <i64 255, i64 255>, <2 x i64> %lo
  %cvt = sitofp <2 x i64> %hi to <2 x double>
  ret <2 x double> %cvt
}
