; ModuleID = 'error.cpp'
source_filename = "error.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.Error = type { i8, i32, i8* }

$_ZN5ErrorC2Ev = comdat any

@.str = private unnamed_addr constant [6 x i8] c"hello\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @_Z7testIntR5Error(%struct.Error* nonnull align 8 dereferenceable(16) %0) #0 {
  %2 = alloca %struct.Error*, align 8
  store %struct.Error* %0, %struct.Error** %2, align 8
  %3 = load %struct.Error*, %struct.Error** %2, align 8
  %4 = getelementptr inbounds %struct.Error, %struct.Error* %3, i32 0, i32 0
  store i8 1, i8* %4, align 8
  %5 = load %struct.Error*, %struct.Error** %2, align 8
  %6 = getelementptr inbounds %struct.Error, %struct.Error* %5, i32 0, i32 1
  store i32 1, i32* %6, align 4
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @_Z10testStringR5Error(%struct.Error* nonnull align 8 dereferenceable(16) %0) #0 {
  %2 = alloca %struct.Error*, align 8
  %3 = alloca i8*, align 8
  store %struct.Error* %0, %struct.Error** %2, align 8
  %4 = load %struct.Error*, %struct.Error** %2, align 8
  %5 = getelementptr inbounds %struct.Error, %struct.Error* %4, i32 0, i32 0
  store i8 2, i8* %5, align 8
  store i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str, i64 0, i64 0), i8** %3, align 8
  %6 = load i8*, i8** %3, align 8
  %7 = load %struct.Error*, %struct.Error** %2, align 8
  %8 = getelementptr inbounds %struct.Error, %struct.Error* %7, i32 0, i32 2
  store i8* %6, i8** %8, align 8
  ret void
}

; Function Attrs: noinline norecurse nounwind optnone uwtable
define dso_local i32 @main() #1 {
  %1 = alloca i32, align 4
  %2 = alloca %struct.Error, align 8
  store i32 0, i32* %1, align 4
  %3 = bitcast %struct.Error* %2 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %3, i8 0, i64 16, i1 false)
  call void @_ZN5ErrorC2Ev(%struct.Error* %2) #3
  call void @_Z7testIntR5Error(%struct.Error* nonnull align 8 dereferenceable(16) %2)
  call void @_Z10testStringR5Error(%struct.Error* nonnull align 8 dereferenceable(16) %2)
  %4 = getelementptr inbounds %struct.Error, %struct.Error* %2, i32 0, i32 1
  %5 = load i32, i32* %4, align 4
  %6 = icmp ne i32 %5, 0
  %7 = zext i1 %6 to i32
  ret i32 %7
}

; Function Attrs: argmemonly nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #2

; Function Attrs: noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZN5ErrorC2Ev(%struct.Error* %0) unnamed_addr #0 comdat align 2 {
  %2 = alloca %struct.Error*, align 8
  store %struct.Error* %0, %struct.Error** %2, align 8
  %3 = load %struct.Error*, %struct.Error** %2, align 8
  %4 = getelementptr inbounds %struct.Error, %struct.Error* %3, i32 0, i32 0
  store i8 0, i8* %4, align 8
  %5 = getelementptr inbounds %struct.Error, %struct.Error* %3, i32 0, i32 1
  store i32 0, i32* %5, align 4
  %6 = getelementptr inbounds %struct.Error, %struct.Error* %3, i32 0, i32 2
  store i8* null, i8** %6, align 8
  ret void
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline norecurse nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { argmemonly nounwind willreturn writeonly }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Ubuntu clang version 11.1.0-6"}
