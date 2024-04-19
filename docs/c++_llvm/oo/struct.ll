; ModuleID = 'struct.cpp'
source_filename = "struct.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.Test = type { i32, i32 }

$_ZN4TestC2Ev = comdat any

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i64 @_Z7getTestv() #0 {
  %1 = alloca %struct.Test, align 4
  call void @_ZN4TestC2Ev(%struct.Test* %1) #2
  %2 = bitcast %struct.Test* %1 to i64*
  %3 = load i64, i64* %2, align 4
  ret i64 %3
}

; Function Attrs: noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZN4TestC2Ev(%struct.Test* %0) unnamed_addr #0 comdat align 2 {
  %2 = alloca %struct.Test*, align 8
  store %struct.Test* %0, %struct.Test** %2, align 8
  %3 = load %struct.Test*, %struct.Test** %2, align 8
  %4 = getelementptr inbounds %struct.Test, %struct.Test* %3, i32 0, i32 0
  store i32 1, i32* %4, align 4
  ret void
}

; Function Attrs: noinline norecurse nounwind optnone uwtable
define dso_local i32 @main() #1 {
  %1 = alloca i32, align 4
  %2 = alloca %struct.Test, align 4
  store i32 0, i32* %1, align 4
  %3 = call i64 @_Z7getTestv()
  %4 = bitcast %struct.Test* %2 to i64*
  store i64 %3, i64* %4, align 4
  %5 = getelementptr inbounds %struct.Test, %struct.Test* %2, i32 0, i32 0
  %6 = load i32, i32* %5, align 4
  ret i32 %6
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline norecurse nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Ubuntu clang version 11.1.0-6"}
