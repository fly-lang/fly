; ModuleID = 'op-logic.cpp'
source_filename = "op-logic.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline norecurse nounwind optnone uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i8, align 1
  %3 = alloca i8, align 1
  %4 = alloca i8, align 1
  store i32 0, i32* %1, align 4
  store i8 1, i8* %2, align 1
  store i8 0, i8* %3, align 1
  store i8 0, i8* %4, align 1
  %5 = load i8, i8* %2, align 1
  %6 = trunc i8 %5 to i1
  %7 = xor i1 %6, true
  %8 = zext i1 %7 to i8
  store i8 %8, i8* %4, align 1
  %9 = load i8, i8* %2, align 1
  %10 = trunc i8 %9 to i1
  br i1 %10, label %11, label %14

11:                                               ; preds = %0
  %12 = load i8, i8* %3, align 1
  %13 = trunc i8 %12 to i1
  br label %14

14:                                               ; preds = %11, %0
  %15 = phi i1 [ false, %0 ], [ %13, %11 ]
  %16 = zext i1 %15 to i8
  store i8 %16, i8* %4, align 1
  %17 = load i8, i8* %2, align 1
  %18 = trunc i8 %17 to i1
  br i1 %18, label %22, label %19

19:                                               ; preds = %14
  %20 = load i8, i8* %3, align 1
  %21 = trunc i8 %20 to i1
  br label %22

22:                                               ; preds = %19, %14
  %23 = phi i1 [ true, %14 ], [ %21, %19 ]
  %24 = zext i1 %23 to i8
  store i8 %24, i8* %4, align 1
  %25 = load i8, i8* %2, align 1
  %26 = trunc i8 %25 to i1
  br i1 %26, label %27, label %30

27:                                               ; preds = %22
  %28 = load i8, i8* %3, align 1
  %29 = trunc i8 %28 to i1
  br i1 %29, label %33, label %30

30:                                               ; preds = %27, %22
  %31 = load i8, i8* %2, align 1
  %32 = trunc i8 %31 to i1
  br label %33

33:                                               ; preds = %30, %27
  %34 = phi i1 [ true, %27 ], [ %32, %30 ]
  %35 = zext i1 %34 to i8
  store i8 %35, i8* %4, align 1
  %36 = load i32, i32* %1, align 4
  ret i32 %36
}

attributes #0 = { noinline norecurse nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
