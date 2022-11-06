; ModuleID = 'struct.cpp'
source_filename = "struct.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.test = type { i32, i32, i32 }

$_ZN4testC2Ev = comdat any

; Function Attrs: noinline norecurse nounwind optnone uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca %struct.test, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  call void @_ZN4testC2Ev(%struct.test* %2) #2
  %6 = getelementptr inbounds %struct.test, %struct.test* %2, i32 0, i32 0
  %7 = load i32, i32* %6, align 4
  store i32 %7, i32* %3, align 4
  %8 = getelementptr inbounds %struct.test, %struct.test* %2, i32 0, i32 0
  %9 = load i32, i32* %8, align 4
  store i32 %9, i32* %4, align 4
  %10 = getelementptr inbounds %struct.test, %struct.test* %2, i32 0, i32 0
  %11 = load i32, i32* %10, align 4
  store i32 %11, i32* %5, align 4
  %12 = getelementptr inbounds %struct.test, %struct.test* %2, i32 0, i32 0
  %13 = load i32, i32* %12, align 4
  ret i32 %13
}

; Function Attrs: noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZN4testC2Ev(%struct.test* %0) unnamed_addr #1 comdat align 2 {
  %2 = alloca %struct.test*, align 8
  store %struct.test* %0, %struct.test** %2, align 8
  %3 = load %struct.test*, %struct.test** %2, align 8
  %4 = getelementptr inbounds %struct.test, %struct.test* %3, i32 0, i32 1
  store i32 2, i32* %4, align 4
  %5 = getelementptr inbounds %struct.test, %struct.test* %3, i32 0, i32 2
  store i32 3, i32* %5, align 4
  ret void
}

attributes #0 = { noinline norecurse nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Debian clang version 11.1.0-++20211011094159+1fdec59bffc1-1~exp1~20211011214627.7"}
