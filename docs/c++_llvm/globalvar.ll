; ModuleID = 'globalvar.cpp'
source_filename = "globalvar.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@a = dso_local global i32 1, align 4
@b = dso_local global float 2.000000e+00, align 4
@c = dso_local global i8 1, align 1

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
