; NOTE: Assertions have been autogenerated by utils/update_test_checks.py
; REQUIRES: aarch64-registered-target
; RUN: opt -passes='lower-matrix-intrinsics' -mtriple=arm64-apple-iphoneos -S < %s | FileCheck %s

define void @transposed_multiply_feeding_dot_product_v4i322(<4 x i32> %a, <4 x i32> %b) {
; CHECK-LABEL: @transposed_multiply_feeding_dot_product_v4i322(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[TMP0:%.*]] = mul <4 x i32> [[A:%.*]], [[B:%.*]]
; CHECK-NEXT:    [[TMP1:%.*]] = call i32 @llvm.vector.reduce.add.v4i32(<4 x i32> [[TMP0]])
; CHECK-NEXT:    [[TMP2:%.*]] = insertelement <1 x i32> poison, i32 [[TMP1]], i64 0
; CHECK-NEXT:    ret void
;
entry:
  %0 = call <4 x i32> @llvm.matrix.transpose.v4i32(<4 x i32> %a, i32 4, i32 1)
  %1 = call <1 x i32> @llvm.matrix.multiply.v1i32.v4i32.v4i32(<4 x i32> %0, <4 x i32> %b, i32 1, i32 4, i32 1)
  ret void
}

define void @transposed_multiply_feeding_dot_produc_v4i32(<4 x i32> %a, <4 x i32> %b, <4 x i32> %c) {
; CHECK-LABEL: @transposed_multiply_feeding_dot_produc_v4i32(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[SPLIT:%.*]] = shufflevector <4 x i32> [[A:%.*]], <4 x i32> poison, <2 x i32> <i32 0, i32 1>
; CHECK-NEXT:    [[SPLIT1:%.*]] = shufflevector <4 x i32> [[A]], <4 x i32> poison, <2 x i32> <i32 2, i32 3>
; CHECK-NEXT:    [[SPLIT2:%.*]] = shufflevector <4 x i32> [[B:%.*]], <4 x i32> poison, <2 x i32> <i32 0, i32 1>
; CHECK-NEXT:    [[SPLIT3:%.*]] = shufflevector <4 x i32> [[B]], <4 x i32> poison, <2 x i32> <i32 2, i32 3>
; CHECK-NEXT:    [[BLOCK:%.*]] = shufflevector <2 x i32> [[SPLIT]], <2 x i32> poison, <2 x i32> <i32 0, i32 1>
; CHECK-NEXT:    [[TMP0:%.*]] = extractelement <2 x i32> [[SPLIT2]], i64 0
; CHECK-NEXT:    [[SPLAT_SPLATINSERT:%.*]] = insertelement <2 x i32> poison, i32 [[TMP0]], i64 0
; CHECK-NEXT:    [[SPLAT_SPLAT:%.*]] = shufflevector <2 x i32> [[SPLAT_SPLATINSERT]], <2 x i32> poison, <2 x i32> zeroinitializer
; CHECK-NEXT:    [[TMP1:%.*]] = mul <2 x i32> [[BLOCK]], [[SPLAT_SPLAT]]
; CHECK-NEXT:    [[BLOCK4:%.*]] = shufflevector <2 x i32> [[SPLIT1]], <2 x i32> poison, <2 x i32> <i32 0, i32 1>
; CHECK-NEXT:    [[TMP2:%.*]] = extractelement <2 x i32> [[SPLIT2]], i64 1
; CHECK-NEXT:    [[SPLAT_SPLATINSERT5:%.*]] = insertelement <2 x i32> poison, i32 [[TMP2]], i64 0
; CHECK-NEXT:    [[SPLAT_SPLAT6:%.*]] = shufflevector <2 x i32> [[SPLAT_SPLATINSERT5]], <2 x i32> poison, <2 x i32> zeroinitializer
; CHECK-NEXT:    [[TMP3:%.*]] = mul <2 x i32> [[BLOCK4]], [[SPLAT_SPLAT6]]
; CHECK-NEXT:    [[TMP4:%.*]] = add <2 x i32> [[TMP1]], [[TMP3]]
; CHECK-NEXT:    [[TMP5:%.*]] = shufflevector <2 x i32> [[TMP4]], <2 x i32> poison, <2 x i32> <i32 0, i32 1>
; CHECK-NEXT:    [[TMP6:%.*]] = shufflevector <2 x i32> poison, <2 x i32> [[TMP5]], <2 x i32> <i32 2, i32 3>
; CHECK-NEXT:    [[BLOCK7:%.*]] = shufflevector <2 x i32> [[SPLIT]], <2 x i32> poison, <2 x i32> <i32 0, i32 1>
; CHECK-NEXT:    [[TMP7:%.*]] = extractelement <2 x i32> [[SPLIT3]], i64 0
; CHECK-NEXT:    [[SPLAT_SPLATINSERT8:%.*]] = insertelement <2 x i32> poison, i32 [[TMP7]], i64 0
; CHECK-NEXT:    [[SPLAT_SPLAT9:%.*]] = shufflevector <2 x i32> [[SPLAT_SPLATINSERT8]], <2 x i32> poison, <2 x i32> zeroinitializer
; CHECK-NEXT:    [[TMP8:%.*]] = mul <2 x i32> [[BLOCK7]], [[SPLAT_SPLAT9]]
; CHECK-NEXT:    [[BLOCK10:%.*]] = shufflevector <2 x i32> [[SPLIT1]], <2 x i32> poison, <2 x i32> <i32 0, i32 1>
; CHECK-NEXT:    [[TMP9:%.*]] = extractelement <2 x i32> [[SPLIT3]], i64 1
; CHECK-NEXT:    [[SPLAT_SPLATINSERT11:%.*]] = insertelement <2 x i32> poison, i32 [[TMP9]], i64 0
; CHECK-NEXT:    [[SPLAT_SPLAT12:%.*]] = shufflevector <2 x i32> [[SPLAT_SPLATINSERT11]], <2 x i32> poison, <2 x i32> zeroinitializer
; CHECK-NEXT:    [[TMP10:%.*]] = mul <2 x i32> [[BLOCK10]], [[SPLAT_SPLAT12]]
; CHECK-NEXT:    [[TMP11:%.*]] = add <2 x i32> [[TMP8]], [[TMP10]]
; CHECK-NEXT:    [[TMP12:%.*]] = shufflevector <2 x i32> [[TMP11]], <2 x i32> poison, <2 x i32> <i32 0, i32 1>
; CHECK-NEXT:    [[TMP13:%.*]] = shufflevector <2 x i32> poison, <2 x i32> [[TMP12]], <2 x i32> <i32 2, i32 3>
; CHECK-NEXT:    [[TMP14:%.*]] = shufflevector <2 x i32> [[TMP6]], <2 x i32> [[TMP13]], <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK-NEXT:    [[TMP15:%.*]] = mul <4 x i32> [[TMP14]], [[C:%.*]]
; CHECK-NEXT:    [[TMP16:%.*]] = call i32 @llvm.vector.reduce.add.v4i32(<4 x i32> [[TMP15]])
; CHECK-NEXT:    [[TMP17:%.*]] = insertelement <1 x i32> poison, i32 [[TMP16]], i64 0
; CHECK-NEXT:    ret void
;
entry:
  %0 = call <4 x i32> @llvm.matrix.multiply.v4i32.v4i32.v4i32(<4 x i32> %a, <4 x i32> %b, i32 2, i32 2, i32 2)
  %1 = call <4 x i32> @llvm.matrix.transpose.v4i32(<4 x i32> %0, i32 4, i32 1)
  %2 = call <1 x i32> @llvm.matrix.multiply.v1i32.v4i32.v4i32(<4 x i32> %1, <4 x i32> %c, i32 1, i32 4, i32 1)
  ret void
}

declare <1 x i32> @llvm.matrix.multiply.v1i32.v4i32.v4i32(<4 x i32>, <4 x i32>, i32, i32, i32)

declare <4 x i32> @llvm.matrix.transpose.v4i32(<4 x i32>, i32 immarg, i32 immarg)

declare <4 x i32> @llvm.matrix.multiply.v4i32.v4i32.v4i32(<4 x i32>, <4 x i32>, i32 immarg, i32 immarg, i32 immarg)

define <1 x i32> @test_load_multiuse(ptr %src, <4 x i32> %b) {
; CHECK-LABEL: @test_load_multiuse(
; CHECK-NEXT:    [[COL_LOAD:%.*]] = load <1 x i32>, ptr [[SRC:%.*]], align 16
; CHECK-NEXT:    [[VEC_GEP:%.*]] = getelementptr i32, ptr [[SRC]], i64 1
; CHECK-NEXT:    [[COL_LOAD1:%.*]] = load <1 x i32>, ptr [[VEC_GEP]], align 4
; CHECK-NEXT:    [[VEC_GEP2:%.*]] = getelementptr i32, ptr [[SRC]], i64 2
; CHECK-NEXT:    [[COL_LOAD3:%.*]] = load <1 x i32>, ptr [[VEC_GEP2]], align 8
; CHECK-NEXT:    [[VEC_GEP4:%.*]] = getelementptr i32, ptr [[SRC]], i64 3
; CHECK-NEXT:    [[COL_LOAD5:%.*]] = load <1 x i32>, ptr [[VEC_GEP4]], align 4
; CHECK-NEXT:    [[TMP1:%.*]] = shufflevector <1 x i32> [[COL_LOAD]], <1 x i32> [[COL_LOAD1]], <2 x i32> <i32 0, i32 1>
; CHECK-NEXT:    [[TMP2:%.*]] = shufflevector <1 x i32> [[COL_LOAD3]], <1 x i32> [[COL_LOAD5]], <2 x i32> <i32 0, i32 1>
; CHECK-NEXT:    [[TMP3:%.*]] = shufflevector <2 x i32> [[TMP1]], <2 x i32> [[TMP2]], <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK-NEXT:    [[TMP4:%.*]] = extractelement <1 x i32> [[COL_LOAD]], i64 0
; CHECK-NEXT:    [[TMP5:%.*]] = insertelement <4 x i32> poison, i32 [[TMP4]], i64 0
; CHECK-NEXT:    [[TMP6:%.*]] = extractelement <1 x i32> [[COL_LOAD1]], i64 0
; CHECK-NEXT:    [[TMP7:%.*]] = insertelement <4 x i32> [[TMP5]], i32 [[TMP6]], i64 1
; CHECK-NEXT:    [[TMP8:%.*]] = extractelement <1 x i32> [[COL_LOAD3]], i64 0
; CHECK-NEXT:    [[TMP9:%.*]] = insertelement <4 x i32> [[TMP7]], i32 [[TMP8]], i64 2
; CHECK-NEXT:    [[TMP10:%.*]] = extractelement <1 x i32> [[COL_LOAD5]], i64 0
; CHECK-NEXT:    [[TMP11:%.*]] = insertelement <4 x i32> [[TMP9]], i32 [[TMP10]], i64 3
; CHECK-NEXT:    call void @use.v4i32(<4 x i32> [[TMP11]])
; CHECK-NEXT:    [[TMP12:%.*]] = mul <4 x i32> [[TMP3]], [[B:%.*]]
; CHECK-NEXT:    [[TMP13:%.*]] = call i32 @llvm.vector.reduce.add.v4i32(<4 x i32> [[TMP12]])
; CHECK-NEXT:    [[TMP14:%.*]] = insertelement <1 x i32> poison, i32 [[TMP13]], i64 0
; CHECK-NEXT:    ret <1 x i32> [[TMP14]]
;
  %l = load <4 x i32>, ptr %src
  %t = call <4 x i32> @llvm.matrix.transpose.v4i32(<4 x i32> %l, i32 1, i32 4)
  call void @use.v4i32(<4 x i32> %t)
  %res = call <1 x i32> @llvm.matrix.multiply.v1i32.v4i32.v4i32(<4 x i32> %l, <4 x i32> %b, i32 1, i32 4, i32 1)
  ret <1 x i32> %res
}

define <1 x i32> @test_builtin_column_major_load_multiuse(ptr %src, <4 x i32> %b) {
; CHECK-LABEL: @test_builtin_column_major_load_multiuse(
; CHECK-NEXT:    [[COL_LOAD:%.*]] = load <1 x i32>, ptr [[SRC:%.*]], align 4
; CHECK-NEXT:    [[VEC_GEP:%.*]] = getelementptr i32, ptr [[SRC]], i64 1
; CHECK-NEXT:    [[COL_LOAD1:%.*]] = load <1 x i32>, ptr [[VEC_GEP]], align 4
; CHECK-NEXT:    [[VEC_GEP2:%.*]] = getelementptr i32, ptr [[SRC]], i64 2
; CHECK-NEXT:    [[COL_LOAD3:%.*]] = load <1 x i32>, ptr [[VEC_GEP2]], align 4
; CHECK-NEXT:    [[VEC_GEP4:%.*]] = getelementptr i32, ptr [[SRC]], i64 3
; CHECK-NEXT:    [[COL_LOAD5:%.*]] = load <1 x i32>, ptr [[VEC_GEP4]], align 4
; CHECK-NEXT:    [[TMP1:%.*]] = shufflevector <1 x i32> [[COL_LOAD]], <1 x i32> [[COL_LOAD1]], <2 x i32> <i32 0, i32 1>
; CHECK-NEXT:    [[TMP2:%.*]] = shufflevector <1 x i32> [[COL_LOAD3]], <1 x i32> [[COL_LOAD5]], <2 x i32> <i32 0, i32 1>
; CHECK-NEXT:    [[TMP3:%.*]] = shufflevector <2 x i32> [[TMP1]], <2 x i32> [[TMP2]], <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK-NEXT:    [[TMP4:%.*]] = extractelement <1 x i32> [[COL_LOAD]], i64 0
; CHECK-NEXT:    [[TMP5:%.*]] = insertelement <4 x i32> poison, i32 [[TMP4]], i64 0
; CHECK-NEXT:    [[TMP6:%.*]] = extractelement <1 x i32> [[COL_LOAD1]], i64 0
; CHECK-NEXT:    [[TMP7:%.*]] = insertelement <4 x i32> [[TMP5]], i32 [[TMP6]], i64 1
; CHECK-NEXT:    [[TMP8:%.*]] = extractelement <1 x i32> [[COL_LOAD3]], i64 0
; CHECK-NEXT:    [[TMP9:%.*]] = insertelement <4 x i32> [[TMP7]], i32 [[TMP8]], i64 2
; CHECK-NEXT:    [[TMP10:%.*]] = extractelement <1 x i32> [[COL_LOAD5]], i64 0
; CHECK-NEXT:    [[TMP11:%.*]] = insertelement <4 x i32> [[TMP9]], i32 [[TMP10]], i64 3
; CHECK-NEXT:    call void @use.v4i32(<4 x i32> [[TMP11]])
; CHECK-NEXT:    [[TMP12:%.*]] = mul <4 x i32> [[TMP3]], [[B:%.*]]
; CHECK-NEXT:    [[TMP13:%.*]] = call i32 @llvm.vector.reduce.add.v4i32(<4 x i32> [[TMP12]])
; CHECK-NEXT:    [[TMP14:%.*]] = insertelement <1 x i32> poison, i32 [[TMP13]], i64 0
; CHECK-NEXT:    ret <1 x i32> [[TMP14]]
;
  %l = call <4 x i32> @llvm.matrix.column.major.load.v4i32.i64(ptr %src, i64 1, i1 false, i32 1, i32 4)
  %t = call <4 x i32> @llvm.matrix.transpose.v4i32(<4 x i32> %l, i32 1, i32 4)
  call void @use.v4i32(<4 x i32> %t)
  %res = call <1 x i32> @llvm.matrix.multiply.v1i32.v4i32.v4i32(<4 x i32> %l, <4 x i32> %b, i32 1, i32 4, i32 1)
  ret <1 x i32> %res
}

declare <4 x i32> @llvm.matrix.column.major.load.v4i32.i64(ptr, i64, i1, i32, i32)

declare void @use.v4i32(<4 x i32>)

define <1 x i32> @test_builtin_column_major_variable_stride_multiuse(ptr %src, <5 x i32> %a, i64 %stride) {
; CHECK-LABEL: @test_builtin_column_major_variable_stride_multiuse(
; CHECK-NEXT:    [[VEC_START:%.*]] = mul i64 0, [[STRIDE:%.*]]
; CHECK-NEXT:    [[VEC_GEP:%.*]] = getelementptr i32, ptr [[SRC:%.*]], i64 [[VEC_START]]
; CHECK-NEXT:    [[COL_LOAD:%.*]] = load <1 x i32>, ptr [[VEC_GEP]], align 4
; CHECK-NEXT:    [[VEC_START1:%.*]] = mul i64 1, [[STRIDE]]
; CHECK-NEXT:    [[VEC_GEP2:%.*]] = getelementptr i32, ptr [[SRC]], i64 [[VEC_START1]]
; CHECK-NEXT:    [[COL_LOAD3:%.*]] = load <1 x i32>, ptr [[VEC_GEP2]], align 4
; CHECK-NEXT:    [[VEC_START4:%.*]] = mul i64 2, [[STRIDE]]
; CHECK-NEXT:    [[VEC_GEP5:%.*]] = getelementptr i32, ptr [[SRC]], i64 [[VEC_START4]]
; CHECK-NEXT:    [[COL_LOAD6:%.*]] = load <1 x i32>, ptr [[VEC_GEP5]], align 4
; CHECK-NEXT:    [[VEC_START7:%.*]] = mul i64 3, [[STRIDE]]
; CHECK-NEXT:    [[VEC_GEP8:%.*]] = getelementptr i32, ptr [[SRC]], i64 [[VEC_START7]]
; CHECK-NEXT:    [[COL_LOAD9:%.*]] = load <1 x i32>, ptr [[VEC_GEP8]], align 4
; CHECK-NEXT:    [[VEC_START10:%.*]] = mul i64 4, [[STRIDE]]
; CHECK-NEXT:    [[VEC_GEP11:%.*]] = getelementptr i32, ptr [[SRC]], i64 [[VEC_START10]]
; CHECK-NEXT:    [[COL_LOAD12:%.*]] = load <1 x i32>, ptr [[VEC_GEP11]], align 4
; CHECK-NEXT:    [[TMP1:%.*]] = shufflevector <1 x i32> [[COL_LOAD]], <1 x i32> [[COL_LOAD3]], <2 x i32> <i32 0, i32 1>
; CHECK-NEXT:    [[TMP2:%.*]] = shufflevector <1 x i32> [[COL_LOAD6]], <1 x i32> [[COL_LOAD9]], <2 x i32> <i32 0, i32 1>
; CHECK-NEXT:    [[TMP3:%.*]] = shufflevector <2 x i32> [[TMP1]], <2 x i32> [[TMP2]], <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK-NEXT:    [[TMP4:%.*]] = shufflevector <1 x i32> [[COL_LOAD12]], <1 x i32> poison, <4 x i32> <i32 0, i32 poison, i32 poison, i32 poison>
; CHECK-NEXT:    [[TMP5:%.*]] = shufflevector <4 x i32> [[TMP3]], <4 x i32> [[TMP4]], <5 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4>
; CHECK-NEXT:    [[TMP6:%.*]] = extractelement <1 x i32> [[COL_LOAD]], i64 0
; CHECK-NEXT:    [[TMP7:%.*]] = insertelement <5 x i32> poison, i32 [[TMP6]], i64 0
; CHECK-NEXT:    [[TMP8:%.*]] = extractelement <1 x i32> [[COL_LOAD3]], i64 0
; CHECK-NEXT:    [[TMP9:%.*]] = insertelement <5 x i32> [[TMP7]], i32 [[TMP8]], i64 1
; CHECK-NEXT:    [[TMP10:%.*]] = extractelement <1 x i32> [[COL_LOAD6]], i64 0
; CHECK-NEXT:    [[TMP11:%.*]] = insertelement <5 x i32> [[TMP9]], i32 [[TMP10]], i64 2
; CHECK-NEXT:    [[TMP12:%.*]] = extractelement <1 x i32> [[COL_LOAD9]], i64 0
; CHECK-NEXT:    [[TMP13:%.*]] = insertelement <5 x i32> [[TMP11]], i32 [[TMP12]], i64 3
; CHECK-NEXT:    [[TMP14:%.*]] = extractelement <1 x i32> [[COL_LOAD12]], i64 0
; CHECK-NEXT:    [[TMP15:%.*]] = insertelement <5 x i32> [[TMP13]], i32 [[TMP14]], i64 4
; CHECK-NEXT:    call void @use.v5i32(<5 x i32> [[TMP15]])
; CHECK-NEXT:    [[TMP16:%.*]] = mul <5 x i32> [[TMP5]], [[A:%.*]]
; CHECK-NEXT:    [[TMP17:%.*]] = call i32 @llvm.vector.reduce.add.v5i32(<5 x i32> [[TMP16]])
; CHECK-NEXT:    [[TMP18:%.*]] = insertelement <1 x i32> poison, i32 [[TMP17]], i64 0
; CHECK-NEXT:    ret <1 x i32> [[TMP18]]
;
  %l = call <5 x i32> @llvm.matrix.column.major.load.v5i32.i64(ptr %src, i64 %stride, i1 false, i32 1, i32 5)
  %t = call <5 x i32> @llvm.matrix.transpose.v5i32(<5 x i32> %l, i32 1, i32 5)
  call void @use.v5i32(<5 x i32> %t)
  %r = call <1 x i32> @llvm.matrix.multiply.v1i32.v5i32.v5i32(<5 x i32> %l, <5 x i32> %a, i32 1, i32 5, i32 1)
  ret <1 x i32> %r
}

declare void @use.v5i32(<5 x i32>)

declare <1 x i32> @llvm.matrix.multiply.v1i32.v5i32.v5i32(<5 x i32>, <5 x i32>, i32 immarg, i32 immarg, i32 immarg) #0

declare <5 x i32> @llvm.matrix.column.major.load.v5i32.i64(ptr nocapture, i64, i1 immarg, i32 immarg, i32 immarg) #1

declare <5 x i32> @llvm.matrix.transpose.v5i32(<5 x i32>, i32 immarg, i32 immarg) #0

define <1 x i32> @test_dot_product_with_transposed_shuffle_op(<4 x i32> %a, <2 x i32> %b) {
; CHECK-LABEL: @test_dot_product_with_transposed_shuffle_op(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[SPLIT:%.*]] = shufflevector <4 x i32> [[A:%.*]], <4 x i32> poison, <2 x i32> <i32 0, i32 1>
; CHECK-NEXT:    [[SPLIT1:%.*]] = shufflevector <4 x i32> [[A]], <4 x i32> poison, <2 x i32> <i32 2, i32 3>
; CHECK-NEXT:    [[TMP0:%.*]] = extractelement <2 x i32> [[SPLIT]], i64 0
; CHECK-NEXT:    [[TMP1:%.*]] = insertelement <2 x i32> poison, i32 [[TMP0]], i64 0
; CHECK-NEXT:    [[TMP2:%.*]] = extractelement <2 x i32> [[SPLIT1]], i64 0
; CHECK-NEXT:    [[TMP3:%.*]] = insertelement <2 x i32> [[TMP1]], i32 [[TMP2]], i64 1
; CHECK-NEXT:    [[TMP4:%.*]] = extractelement <2 x i32> [[SPLIT]], i64 1
; CHECK-NEXT:    [[TMP5:%.*]] = insertelement <2 x i32> poison, i32 [[TMP4]], i64 0
; CHECK-NEXT:    [[TMP6:%.*]] = extractelement <2 x i32> [[SPLIT1]], i64 1
; CHECK-NEXT:    [[TMP7:%.*]] = insertelement <2 x i32> [[TMP5]], i32 [[TMP6]], i64 1
; CHECK-NEXT:    [[TMP8:%.*]] = shufflevector <2 x i32> [[TMP3]], <2 x i32> [[TMP7]], <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK-NEXT:    [[SHUFFLE:%.*]] = shufflevector <4 x i32> [[TMP8]], <4 x i32> zeroinitializer, <2 x i32> <i32 0, i32 1>
; CHECK-NEXT:    [[TMP9:%.*]] = mul <2 x i32> [[SHUFFLE]], [[B:%.*]]
; CHECK-NEXT:    [[TMP10:%.*]] = call i32 @llvm.vector.reduce.add.v2i32(<2 x i32> [[TMP9]])
; CHECK-NEXT:    [[TMP11:%.*]] = insertelement <1 x i32> poison, i32 [[TMP10]], i64 0
; CHECK-NEXT:    ret <1 x i32> [[TMP11]]
;
entry:
  %t.a = tail call <4 x i32> @llvm.matrix.transpose.v4i32(<4 x i32> %a, i32 2, i32 2)
  %shuffle = shufflevector <4 x i32> %t.a, <4 x i32> zeroinitializer, <2 x i32> <i32 0, i32 1>
  %t.shuffle = call <2 x i32> @llvm.matrix.transpose.v2i32(<2 x i32> %shuffle, i32 2, i32 1)
  %m = call <1 x i32> @llvm.matrix.multiply.v1i32.v2i32.v2i32(<2 x i32> %t.shuffle, <2 x i32> %b, i32 1, i32 2, i32 1)
  ret <1 x i32> %m
}

declare <2 x i32> @llvm.matrix.transpose.v2i32(<2 x i32>, i32 immarg, i32 immarg)
