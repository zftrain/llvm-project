; NOTE: Assertions have been autogenerated by utils/update_test_checks.py UTC_ARGS: --version 2
; RUN: opt -mtriple=x86_64-unknown-linux-gnu -passes=slp-vectorizer -S < %s | FileCheck %s

define void @test() {
; CHECK-LABEL: define void @test() {
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[IDX2:%.*]] = getelementptr [1000 x i64], ptr null, i64 0, i64 7
; CHECK-NEXT:    [[TMP0:%.*]] = load <8 x i64>, ptr [[IDX2]], align 8
; CHECK-NEXT:    [[TMP1:%.*]] = load <8 x i64>, ptr [[IDX2]], align 8
; CHECK-NEXT:    [[TMP3:%.*]] = load i64, ptr null, align 8
; CHECK-NEXT:    br label [[LOOP:%.*]]
; CHECK:       loop:
; CHECK-NEXT:    [[PHI1:%.*]] = phi i64 [ 0, [[ENTRY:%.*]] ], [ [[OP_RDX25:%.*]], [[LOOP]] ]
; CHECK-NEXT:    [[TMP6:%.*]] = phi <8 x i64> [ [[TMP0]], [[ENTRY]] ], [ [[TMP1]], [[LOOP]] ]
; CHECK-NEXT:    [[TMP7:%.*]] = mul <8 x i64> [[TMP6]], splat (i64 4)
; CHECK-NEXT:    [[TMP5:%.*]] = mul <8 x i64> [[TMP1]], splat (i64 2)
; CHECK-NEXT:    [[RDX_OP:%.*]] = add <8 x i64> [[TMP7]], [[TMP5]]
; CHECK-NEXT:    [[OP_RDX16:%.*]] = call i64 @llvm.vector.reduce.add.v8i64(<8 x i64> [[RDX_OP]])
; CHECK-NEXT:    [[OP_RDX25]] = add i64 [[OP_RDX16]], [[TMP3]]
; CHECK-NEXT:    br label [[LOOP]]
;
entry:
  %idx1 = getelementptr [1000 x i64], ptr null, i64 0, i64 9
  %idx2 = getelementptr [1000 x i64], ptr null, i64 0, i64 7
  %ld1 = load i64, ptr %idx2, align 8
  %idx3 = getelementptr [1000 x i64], ptr null, i64 0, i64 8
  %ld2 = load i64, ptr %idx3, align 16
  %ld3 = load i64, ptr %idx1, align 8
  %idx4 = getelementptr [1000 x i64], ptr null, i64 0, i64 10
  %ld4 = load i64, ptr %idx4, align 16
  %idx5 = getelementptr [1000 x i64], ptr null, i64 0, i64 11
  %ld5 = load i64, ptr %idx5, align 8
  %idx6 = getelementptr [1000 x i64], ptr null, i64 0, i64 12
  %ld6 = load i64, ptr %idx6, align 16
  %idx7 = getelementptr [1000 x i64], ptr null, i64 0, i64 13
  %ld7 = load i64, ptr %idx7, align 8
  %idx8 = getelementptr [1000 x i64], ptr null, i64 0, i64 14
  %ld8 = load i64, ptr %idx8, align 16
  %0 = load i64, ptr %idx2, align 8
  %1 = load i64, ptr %idx3, align 16
  %2 = load i64, ptr %idx1, align 8
  %3 = load i64, ptr %idx4, align 16
  %4 = load i64, ptr %idx5, align 8
  %5 = load i64, ptr %idx6, align 16
  %6 = load i64, ptr %idx7, align 8
  %7 = load i64, ptr %idx8, align 16
  %8 = load i64, ptr null, align 8
  br label %loop

loop:                 ; preds = %loop, %entry
  %9 = phi i64 [ %ld8, %entry ], [ %7, %loop ]
  %10 = phi i64 [ %ld7, %entry ], [ %6, %loop ]
  %11 = phi i64 [ %ld6, %entry ], [ %5, %loop ]
  %12 = phi i64 [ %ld5, %entry ], [ %4, %loop ]
  %13 = phi i64 [ %ld4, %entry ], [ %3, %loop ]
  %14 = phi i64 [ %ld3, %entry ], [ %2, %loop ]
  %15 = phi i64 [ %ld2, %entry ], [ %1, %loop ]
  %16 = phi i64 [ %ld1, %entry ], [ %0, %loop ]
  %phi1 = phi i64 [ 0, %entry ], [ %64, %loop ]
  %17 = add i64 %16, %15
  %18 = add i64 %17, %14
  %19 = add i64 %18, %13
  %20 = add i64 %19, %12
  %21 = add i64 %20, %11
  %22 = add i64 %21, %10
  %23 = add i64 %22, %9
  %24 = add i64 %23, %16
  %25 = add i64 %24, %15
  %26 = add i64 %25, %14
  %27 = add i64 %26, %13
  %28 = add i64 %27, %12
  %29 = add i64 %28, %11
  %30 = add i64 %29, %10
  %31 = add i64 %30, %9
  %32 = add i64 %31, %16
  %33 = add i64 %32, %15
  %34 = add i64 %33, %14
  %35 = add i64 %34, %13
  %36 = add i64 %35, %12
  %37 = add i64 %36, %11
  %38 = add i64 %37, %10
  %39 = add i64 %38, %9
  %40 = add i64 %39, %16
  %41 = add i64 %40, %15
  %42 = add i64 %41, %14
  %43 = add i64 %42, %13
  %44 = add i64 %43, %12
  %45 = add i64 %44, %11
  %46 = add i64 %45, %10
  %47 = add i64 %46, %9
  %48 = add i64 %47, %0
  %49 = add i64 %48, %1
  %50 = add i64 %49, %2
  %51 = add i64 %50, %3
  %52 = add i64 %51, %4
  %53 = add i64 %52, %5
  %54 = add i64 %53, %6
  %55 = add i64 %54, %7
  %56 = add i64 %55, %8
  %57 = add i64 %56, %0
  %58 = add i64 %57, %1
  %59 = add i64 %58, %2
  %60 = add i64 %59, %3
  %61 = add i64 %60, %4
  %62 = add i64 %61, %5
  %63 = add i64 %62, %6
  %64 = add i64 %63, %7
  br label %loop
}
