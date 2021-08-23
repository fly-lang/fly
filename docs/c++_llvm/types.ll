; ModuleID = 'types.cpp'
source_filename = "types.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @_Z4testv() #0 {
  %1 = alloca i32, align 4
  %2 = alloca float, align 4
  %3 = alloca i8, align 1
  %4 = alloca i64, align 8
  %5 = alloca float, align 4
  %6 = alloca i8, align 1
  %7 = load i32, i32* %1, align 4
  %8 = zext i32 %7 to i64
  store i64 %8, i64* %4, align 8
  %9 = load i8, i8* %3, align 1
  %10 = trunc i8 %9 to i1
  %11 = zext i1 %10 to i64
  store i64 %11, i64* %4, align 8
  %12 = load i32, i32* %1, align 4
  %13 = uitofp i32 %12 to float
  store float %13, float* %5, align 4
  %14 = load i8, i8* %3, align 1
  %15 = trunc i8 %14 to i1
  %16 = uitofp i1 %15 to float
  store float %16, float* %5, align 4
  %17 = load i32, i32* %1, align 4
  %18 = icmp ne i32 %17, 0
  %19 = zext i1 %18 to i8
  store i8 %19, i8* %6, align 1
  %20 = load float, float* %2, align 4
  %21 = fcmp une float %20, 0.000000e+00
  %22 = zext i1 %21 to i8
  store i8 %22, i8* %6, align 1
  ret i32 1
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
