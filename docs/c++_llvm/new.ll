; ModuleID = 'new.c'
source_filename = "new.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.Test = type { i32, i32 }

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca %struct.Test*, align 8
  store i32 0, i32* %1, align 4
  %3 = call noalias i8* @malloc(i64 8) #2
  %4 = bitcast i8* %3 to %struct.Test*
  store %struct.Test* %4, %struct.Test** %2, align 8
  %5 = load %struct.Test*, %struct.Test** %2, align 8
  %6 = getelementptr inbounds %struct.Test, %struct.Test* %5, i32 0, i32 0
  store i32 10, i32* %6, align 4
  %7 = load %struct.Test*, %struct.Test** %2, align 8
  %8 = getelementptr inbounds %struct.Test, %struct.Test* %7, i32 0, i32 1
  store i32 20, i32* %8, align 4
  %9 = load %struct.Test*, %struct.Test** %2, align 8
  %10 = bitcast %struct.Test* %9 to i8*
  call void @free(i8* %10) #2
  ret i32 0
}

; Function Attrs: nounwind
declare dso_local noalias i8* @malloc(i64) #1

; Function Attrs: nounwind
declare dso_local void @free(i8*) #1

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Ubuntu clang version 11.1.0-6"}
