; ModuleID = 'op-comparison.cpp'
source_filename = "op-comparison.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline norecurse nounwind optnone uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca float, align 4
  %4 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  store i32 1, i32* %2, align 4
  store float 2.000000e+00, float* %3, align 4
  %5 = load float, float* %3, align 4
  %6 = load i32, i32* %2, align 4
  %7 = sitofp i32 %6 to float
  %8 = fcmp oeq float %5, %7
  %9 = zext i1 %8 to i32
  store i32 %9, i32* %4, align 4
  %10 = load i32, i32* %2, align 4
  %11 = sitofp i32 %10 to float
  %12 = load float, float* %3, align 4
  %13 = fcmp une float %11, %12
  %14 = zext i1 %13 to i32
  store i32 %14, i32* %4, align 4
  %15 = load i32, i32* %2, align 4
  %16 = sitofp i32 %15 to float
  %17 = load float, float* %3, align 4
  %18 = fcmp ole float %16, %17
  %19 = zext i1 %18 to i32
  store i32 %19, i32* %4, align 4
  %20 = load i32, i32* %2, align 4
  %21 = sitofp i32 %20 to float
  %22 = load float, float* %3, align 4
  %23 = fcmp oge float %21, %22
  %24 = zext i1 %23 to i32
  store i32 %24, i32* %4, align 4
  %25 = load i32, i32* %2, align 4
  %26 = sitofp i32 %25 to float
  %27 = load float, float* %3, align 4
  %28 = fcmp ogt float %26, %27
  %29 = zext i1 %28 to i32
  store i32 %29, i32* %4, align 4
  %30 = load i32, i32* %2, align 4
  %31 = sitofp i32 %30 to float
  %32 = load float, float* %3, align 4
  %33 = fcmp olt float %31, %32
  %34 = zext i1 %33 to i32
  store i32 %34, i32* %4, align 4
  %35 = load i32, i32* %2, align 4
  %36 = sitofp i32 %35 to float
  %37 = load float, float* %3, align 4
  %38 = fcmp oeq float %36, %37
  br i1 %38, label %44, label %39

39:                                               ; preds = %0
  %40 = load i32, i32* %2, align 4
  %41 = sitofp i32 %40 to float
  %42 = load float, float* %3, align 4
  %43 = fcmp une float %41, %42
  br label %44

44:                                               ; preds = %39, %0
  %45 = phi i1 [ true, %0 ], [ %43, %39 ]
  %46 = zext i1 %45 to i32
  store i32 %46, i32* %4, align 4
  %47 = load i32, i32* %1, align 4
  ret i32 %47
}

attributes #0 = { noinline norecurse nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
