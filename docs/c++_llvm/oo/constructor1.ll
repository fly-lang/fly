; ModuleID = 'constructor1.cpp'
source_filename = "constructor1.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%class.Test = type { i32, float }

$_ZN4TestC2Ei = comdat any

$_ZN4Test4getAEv = comdat any

; Function Attrs: noinline norecurse optnone uwtable
define dso_local i32 @main() #0 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %1 = alloca i32, align 4
  %2 = alloca %class.Test*, align 8
  %3 = alloca i8*, align 8
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  %6 = call noalias nonnull i8* @_Znwm(i64 8) #4
  %7 = bitcast i8* %6 to %class.Test*
  invoke void @_ZN4TestC2Ei(%class.Test* %7, i32 1)
          to label %8 unwind label %23

8:                                                ; preds = %0
  store %class.Test* %7, %class.Test** %2, align 8
  %9 = load %class.Test*, %class.Test** %2, align 8
  %10 = call i32 @_ZN4Test4getAEv(%class.Test* %9)
  store i32 %10, i32* %5, align 4
  %11 = load %class.Test*, %class.Test** %2, align 8
  %12 = call i32 @_ZN4Test4getAEv(%class.Test* %11)
  store i32 %12, i32* %5, align 4
  %13 = load %class.Test*, %class.Test** %2, align 8
  %14 = getelementptr inbounds %class.Test, %class.Test* %13, i32 0, i32 1
  store float 4.400000e+01, float* %14, align 4
  %15 = load %class.Test*, %class.Test** %2, align 8
  %16 = getelementptr inbounds %class.Test, %class.Test* %15, i32 0, i32 1
  store float 2.200000e+01, float* %16, align 4
  %17 = load %class.Test*, %class.Test** %2, align 8
  %18 = icmp eq %class.Test* %17, null
  br i1 %18, label %21, label %19

19:                                               ; preds = %8
  %20 = bitcast %class.Test* %17 to i8*
  call void @_ZdlPv(i8* %20) #5
  br label %21

21:                                               ; preds = %19, %8
  %22 = load i32, i32* %5, align 4
  ret i32 %22

23:                                               ; preds = %0
  %24 = landingpad { i8*, i32 }
          cleanup
  %25 = extractvalue { i8*, i32 } %24, 0
  store i8* %25, i8** %3, align 8
  %26 = extractvalue { i8*, i32 } %24, 1
  store i32 %26, i32* %4, align 4
  call void @_ZdlPv(i8* %6) #5
  br label %27

27:                                               ; preds = %23
  %28 = load i8*, i8** %3, align 8
  %29 = load i32, i32* %4, align 4
  %30 = insertvalue { i8*, i32 } undef, i8* %28, 0
  %31 = insertvalue { i8*, i32 } %30, i32 %29, 1
  resume { i8*, i32 } %31
}

; Function Attrs: nobuiltin allocsize(0)
declare dso_local nonnull i8* @_Znwm(i64) #1

; Function Attrs: noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZN4TestC2Ei(%class.Test* %0, i32 %1) unnamed_addr #2 comdat align 2 {
  %3 = alloca %class.Test*, align 8
  %4 = alloca i32, align 4
  store %class.Test* %0, %class.Test** %3, align 8
  store i32 %1, i32* %4, align 4
  %5 = load %class.Test*, %class.Test** %3, align 8
  %6 = load i32, i32* %4, align 4
  %7 = getelementptr inbounds %class.Test, %class.Test* %5, i32 0, i32 0
  store i32 %6, i32* %7, align 4
  %8 = getelementptr inbounds %class.Test, %class.Test* %5, i32 0, i32 1
  store float 3.300000e+01, float* %8, align 4
  ret void
}

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: nobuiltin nounwind
declare dso_local void @_ZdlPv(i8*) #3

; Function Attrs: noinline nounwind optnone uwtable
define linkonce_odr dso_local i32 @_ZN4Test4getAEv(%class.Test* %0) #2 comdat align 2 {
  %2 = alloca %class.Test*, align 8
  store %class.Test* %0, %class.Test** %2, align 8
  %3 = load %class.Test*, %class.Test** %2, align 8
  %4 = getelementptr inbounds %class.Test, %class.Test* %3, i32 0, i32 0
  %5 = load i32, i32* %4, align 4
  ret i32 %5
}

attributes #0 = { noinline norecurse optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nobuiltin allocsize(0) "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nobuiltin nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { builtin allocsize(0) }
attributes #5 = { builtin nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Ubuntu clang version 11.1.0-6"}
