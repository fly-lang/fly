; ModuleID = 'op-ternary.cpp'
source_filename = "op-ternary.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline norecurse nounwind optnone uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i8, align 1
  %3 = alloca i8, align 1
  %4 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  store i8 1, i8* %2, align 1
  store i8 0, i8* %3, align 1
  %5 = load i8, i8* %2, align 1
  %6 = trunc i8 %5 to i1
  %7 = zext i1 %6 to i32
  %8 = load i8, i8* %3, align 1
  %9 = trunc i8 %8 to i1
  %10 = zext i1 %9 to i32
  %11 = icmp eq i32 %7, %10
  br i1 %11, label %12, label %15

12:                                               ; preds = %0
  %13 = load i8, i8* %2, align 1
  %14 = trunc i8 %13 to i1
  br label %18

15:                                               ; preds = %0
  %16 = load i8, i8* %3, align 1
  %17 = trunc i8 %16 to i1
  br label %18

18:                                               ; preds = %15, %12
  %19 = phi i1 [ %14, %12 ], [ %17, %15 ]
  %20 = zext i1 %19 to i32
  store i32 %20, i32* %4, align 4
  %21 = load i32, i32* %1, align 4
  ret i32 %21
}

attributes #0 = { noinline norecurse nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Debian clang version 11.1.0-++20211011094159+1fdec59bffc1-1~exp1~20211011214627.7"}
