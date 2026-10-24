; ModuleID = 'test/test22_pointer_types.ckp'
source_filename = "test/test22_pointer_types.ckp"

%struct.Point = type { i32, i32 }
%class.Grid = type { [2 x [3 x i32]], ptr, ptr }

@global_4deep = global ptr null
@global_10pointBlock = global ptr null
@global_5names = internal global ptr null

define ptr @global_7pickRow_PPPiPPcPFiiiE(ptr %table, ptr %argv, ptr %apply) {
entry:
  %table.addr = alloca ptr
  %argv.addr = alloca ptr
  %apply.addr = alloca ptr
  store ptr %table, ptr %table.addr
  store ptr %argv, ptr %argv.addr
  store ptr %apply, ptr %apply.addr
  %t0 = load ptr, ptr %table.addr
  %t1 = load ptr, ptr %t0
  ret ptr %t1
}

define i32 @global_5twice_ii(i32 %a, i32 %b) {
entry:
  %a.addr = alloca i32
  %b.addr = alloca i32
  store i32 %a, ptr %a.addr
  store i32 %b, ptr %b.addr
  %t0 = load i32, ptr %a.addr
  %t1 = load i32, ptr %b.addr
  %t2 = mul i32 %t0, %t1
  ret i32 %t2
}

define i32 @main() {
entry:
  %value = alloca i32
  %p = alloca ptr
  %pp = alloca ptr
  %ppp = alloca ptr
  %read = alloca i32
  %block = alloca ptr
  %fp = alloca ptr
  %fpp = alloca ptr
  %called = alloca i32
  %grid = alloca [2 x [3 x i32]]
  %row = alloca ptr
  %element = alloca i32
  %alias = alloca ptr
  %origin = alloca %struct.Point
  %one = alloca ptr
  %two = alloca ptr
  %viaTwo = alloca i32
  %heapRows = alloca ptr
  %heapPoints = alloca ptr
  %heapBlock = alloca ptr
  %line = alloca [4 x i32]
  %first = alloca ptr
  %third = alloca ptr
  %gap = alloca i32
  %empty = alloca ptr
  %same = alloca i1
  %any = alloca ptr
  %alsoAny = alloca ptr
  %g = alloca %class.Grid
  %fromMember = alloca i32
  store i32 7, ptr %value
  store ptr %value, ptr %p
  store ptr %p, ptr %pp
  store ptr %pp, ptr %ppp
  %t0 = load ptr, ptr %ppp
  %t1 = load ptr, ptr %t0
  %t2 = load ptr, ptr %t1
  %t3 = load i32, ptr %t2
  store i32 %t3, ptr %read
  store ptr @global_5twice_ii, ptr %fp
  store ptr %fp, ptr %fpp
  %t4 = load ptr, ptr %fpp
  %t5 = load ptr, ptr %t4
  %t6 = call i32 %t5(i32 2, i32 3)
  %t7 = load ptr, ptr %fp
  %t8 = call i32 %t7(i32 4, i32 5)
  %t9 = add i32 %t6, %t8
  store i32 %t9, ptr %called
  %t10 = getelementptr inbounds [2 x [3 x i32]], ptr %grid, i64 0, i64 0
  store ptr %t10, ptr %row
  %t11 = load ptr, ptr %row
  %t12 = getelementptr inbounds [3 x i32], ptr %t11, i32 1
  %t13 = getelementptr inbounds [3 x i32], ptr %t12, i64 0, i64 0
  %t14 = getelementptr inbounds i32, ptr %t13, i32 2
  %t15 = load i32, ptr %t14
  store i32 %t15, ptr %element
  store ptr %grid, ptr %alias
  %t16 = load ptr, ptr %alias
  %t17 = getelementptr inbounds [2 x [3 x i32]], ptr %t16, i64 0, i64 0
  %t18 = getelementptr inbounds [3 x i32], ptr %t17, i32 0
  %t19 = getelementptr inbounds [3 x i32], ptr %t18, i64 0, i64 0
  %t20 = getelementptr inbounds i32, ptr %t19, i32 0
  store i32 1, ptr %t20
  store ptr %origin, ptr %one
  store ptr %one, ptr %two
  %t21 = load ptr, ptr %two
  %t22 = load ptr, ptr %t21
  %t23 = getelementptr inbounds %struct.Point, ptr %t22, i32 0, i32 0
  %t24 = load i32, ptr %t23
  %t25 = load ptr, ptr %two
  %t26 = load ptr, ptr %t25
  %t27 = getelementptr inbounds %struct.Point, ptr %t26, i32 0, i32 1
  %t28 = load i32, ptr %t27
  %t29 = add i32 %t24, %t28
  store i32 %t29, ptr %viaTwo
  %t30 = sext i32 4 to i64
  %t31 = getelementptr ptr, ptr null, i64 %t30
  %t32 = ptrtoint ptr %t31 to i64
  %t33 = call ptr @malloc(i64 %t32)
  store ptr %t33, ptr %heapRows
  %t34 = sext i32 2 to i64
  %t35 = getelementptr ptr, ptr null, i64 %t34
  %t36 = ptrtoint ptr %t35 to i64
  %t37 = call ptr @malloc(i64 %t36)
  store ptr %t37, ptr %heapPoints
  %t38 = sext i32 2 to i64
  %t39 = getelementptr [3 x i32], ptr null, i64 %t38
  %t40 = ptrtoint ptr %t39 to i64
  %t41 = call ptr @malloc(i64 %t40)
  store ptr %t41, ptr %heapBlock
  %t42 = load ptr, ptr %p
  %t43 = load ptr, ptr %heapRows
  %t44 = getelementptr inbounds ptr, ptr %t43, i32 0
  store ptr %t42, ptr %t44
  %t45 = getelementptr inbounds [4 x i32], ptr %line, i64 0, i64 0
  store ptr %t45, ptr %first
  %t46 = load ptr, ptr %first
  %t47 = getelementptr inbounds i32, ptr %t46, i32 2
  store ptr %t47, ptr %third
  %t48 = load ptr, ptr %third
  %t49 = load ptr, ptr %first
  %t50 = ptrtoint ptr %t48 to i64
  %t51 = ptrtoint ptr %t49 to i64
  %t52 = sub i64 %t50, %t51
  %t53 = getelementptr i32, ptr null, i64 1
  %t54 = ptrtoint ptr %t53 to i64
  %t55 = sdiv exact i64 %t52, %t54
  %t56 = trunc i64 %t55 to i32
  store i32 %t56, ptr %gap
  store ptr null, ptr %empty
  %t57 = load ptr, ptr %empty
  %t58 = load ptr, ptr %p
  %t59 = icmp eq ptr %t57, %t58
  store i1 %t59, ptr %same
  %t60 = load ptr, ptr %p
  store ptr %t60, ptr %any
  %t61 = load ptr, ptr %one
  store ptr %t61, ptr %alsoAny
  %t62 = getelementptr inbounds [2 x [3 x i32]], ptr %grid, i64 0, i64 0
  %t63 = getelementptr inbounds %class.Grid, ptr %g, i32 0, i32 2
  store ptr %t62, ptr %t63
  %t64 = load ptr, ptr %heapRows
  %t65 = getelementptr inbounds %class.Grid, ptr %g, i32 0, i32 1
  store ptr %t64, ptr %t65
  %t66 = getelementptr inbounds %class.Grid, ptr %g, i32 0, i32 2
  %t67 = load ptr, ptr %t66
  %t68 = getelementptr inbounds [3 x i32], ptr %t67, i32 0
  %t69 = getelementptr inbounds [3 x i32], ptr %t68, i64 0, i64 0
  %t70 = getelementptr inbounds i32, ptr %t69, i32 1
  %t71 = load i32, ptr %t70
  %t72 = getelementptr inbounds %class.Grid, ptr %g, i32 0, i32 0
  %t73 = getelementptr inbounds [2 x [3 x i32]], ptr %t72, i64 0, i64 0
  %t74 = getelementptr inbounds [3 x i32], ptr %t73, i32 1
  %t75 = getelementptr inbounds [3 x i32], ptr %t74, i64 0, i64 0
  %t76 = getelementptr inbounds i32, ptr %t75, i32 2
  %t77 = load i32, ptr %t76
  %t78 = add i32 %t71, %t77
  store i32 %t78, ptr %fromMember
  %t79 = load ptr, ptr %heapRows
  call void @free(ptr %t79)
  %t80 = load ptr, ptr %heapPoints
  call void @free(ptr %t80)
  %t81 = load ptr, ptr %heapBlock
  call void @free(ptr %t81)
  %t82 = load ptr, ptr @global_5names
  call void @free(ptr %t82)
  %t83 = load i32, ptr %read
  %t84 = load i32, ptr %called
  %t85 = add i32 %t83, %t84
  %t86 = load i32, ptr %element
  %t87 = add i32 %t85, %t86
  %t88 = load i32, ptr %viaTwo
  %t89 = add i32 %t87, %t88
  %t90 = load i32, ptr %gap
  %t91 = add i32 %t89, %t90
  %t92 = load i32, ptr %fromMember
  %t93 = add i32 %t91, %t92
  %t94 = load i1, ptr %same
  %t95 = zext i1 %t94 to i32
  %t96 = add i32 %t93, %t95
  ret i32 %t96
}

declare ptr @malloc(i64)

declare void @free(ptr)
