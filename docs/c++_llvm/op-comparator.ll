; ModuleID = 'op-comparator.cpp'
source_filename = "op-comparator.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline norecurse nounwind optnone uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i8, align 1

  ; a == b
  %4 = load i32, i32* %1, align 4
  %5 = load i32, i32* %2, align 4
  %6 = icmp eq i32 %4, %5
  %7 = zext i1 %6 to i8
  store i8 %7, i8* %3, align 1

  ; a != b
  %8 = load i32, i32* %1, align 4
  %9 = load i32, i32* %2, align 4
  %10 = icmp ne i32 %8, %9
  %11 = zext i1 %10 to i8
  store i8 %11, i8* %3, align 1

  ; a > b
  %12 = load i32, i32* %1, align 4
  %13 = load i32, i32* %2, align 4
  %14 = icmp sgt i32 %12, %13
  %15 = zext i1 %14 to i8
  store i8 %15, i8* %3, align 1

  ; a >= b
  %16 = load i32, i32* %1, align 4
  %17 = load i32, i32* %2, align 4
  %18 = icmp sge i32 %16, %17
  %19 = zext i1 %18 to i8
  store i8 %19, i8* %3, align 1

  ; a < b
  %20 = load i32, i32* %1, align 4
  %21 = load i32, i32* %2, align 4
  %22 = icmp slt i32 %20, %21
  %23 = zext i1 %22 to i8
  store i8 %23, i8* %3, align 1

  ; a <= b
  %24 = load i32, i32* %1, align 4
  %25 = load i32, i32* %2, align 4
  %26 = icmp sle i32 %24, %25
  %27 = zext i1 %26 to i8
  store i8 %27, i8* %3, align 1

  ; (a == b) == (a != b)
  %28 = load i32, i32* %1, align 4
  %29 = load i32, i32* %2, align 4
  %30 = icmp eq i32 %28, %29
  %31 = zext i1 %30 to i32
  %32 = load i32, i32* %1, align 4
  %33 = load i32, i32* %2, align 4
  %34 = icmp ne i32 %32, %33
  %35 = zext i1 %34 to i32
  %36 = icmp eq i32 %31, %35
  %37 = zext i1 %36 to i8
  store i8 %37, i8* %3, align 1

  ret i32 0
}

attributes #0 = { noinline norecurse nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Debian clang version 11.1.0-++20211011094159+1fdec59bffc1-1~exp1~20211011214627.7"}
