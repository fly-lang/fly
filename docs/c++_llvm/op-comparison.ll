; ModuleID = 'op-comparison.cpp'
source_filename = "op-comparison.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline norecurse nounwind optnone uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca float, align 4
  %3 = alloca i32, align 4
  store i32 1, i32* %1, align 4
  store float 2.000000e+00, float* %2, align 4
  %4 = load float, float* %2, align 4
  %5 = load i32, i32* %1, align 4
  %6 = sitofp i32 %5 to float
  %7 = fcmp oeq float %4, %6
  %8 = zext i1 %7 to i32
  store i32 %8, i32* %3, align 4
  %9 = load i32, i32* %1, align 4
  %10 = sitofp i32 %9 to float
  %11 = load float, float* %2, align 4
  %12 = fcmp une float %10, %11
  %13 = zext i1 %12 to i32
  store i32 %13, i32* %3, align 4
  %14 = load i32, i32* %1, align 4
  %15 = sitofp i32 %14 to float
  %16 = load float, float* %2, align 4
  %17 = fcmp ole float %15, %16
  %18 = zext i1 %17 to i32
  store i32 %18, i32* %3, align 4
  %19 = load i32, i32* %1, align 4
  %20 = sitofp i32 %19 to float
  %21 = load float, float* %2, align 4
  %22 = fcmp oge float %20, %21
  %23 = zext i1 %22 to i32
  store i32 %23, i32* %3, align 4
  %24 = load i32, i32* %1, align 4
  %25 = sitofp i32 %24 to float
  %26 = load float, float* %2, align 4
  %27 = fcmp ogt float %25, %26
  %28 = zext i1 %27 to i32
  store i32 %28, i32* %3, align 4
  %29 = load i32, i32* %1, align 4
  %30 = sitofp i32 %29 to float
  %31 = load float, float* %2, align 4
  %32 = fcmp olt float %30, %31
  %33 = zext i1 %32 to i32
  store i32 %33, i32* %3, align 4
  %34 = load i32, i32* %1, align 4
  %35 = sitofp i32 %34 to float
  %36 = load float, float* %2, align 4
  %37 = fcmp oeq float %35, %36
  %38 = zext i1 %37 to i32
  %39 = load i32, i32* %1, align 4
  %40 = sitofp i32 %39 to float
  %41 = load float, float* %2, align 4
  %42 = fcmp une float %40, %41
  %43 = zext i1 %42 to i32
  %44 = icmp eq i32 %38, %43
  %45 = zext i1 %44 to i32
  store i32 %45, i32* %3, align 4
  ret i32 0
}

attributes #0 = { noinline norecurse nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
