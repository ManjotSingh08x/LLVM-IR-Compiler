; ModuleID = 'test/test25_ir_arrays_pointers.ckp'
source_filename = "test/test25_ir_arrays_pointers.ckp"

%struct.Point = type { i32, i32 }

@.str = private unnamed_addr constant [13 x i8] c"%d %d %d %s\0A\00"
@.str.1 = private unnamed_addr constant [7 x i8] c"%d %d\0A\00"
@.str.2 = private unnamed_addr constant [10 x i8] c"%d %d %d\0A\00"
@.str.3 = private unnamed_addr constant [4 x i8] c"%d\0A\00"

declare i32 @printf(ptr, ...)

define void @global_4swap_RiRi(ptr %a, ptr %b) {
entry:
  %a.addr = alloca ptr
  %b.addr = alloca ptr
  %t = alloca i32
  store ptr %a, ptr %a.addr
  store ptr %b, ptr %b.addr
  %t0 = load ptr, ptr %a.addr
  %t1 = load i32, ptr %t0
  store i32 %t1, ptr %t
  %t2 = load ptr, ptr %b.addr
  %t3 = load i32, ptr %t2
  %t4 = load ptr, ptr %a.addr
  store i32 %t3, ptr %t4
  %t5 = load i32, ptr %t
  %t6 = load ptr, ptr %b.addr
  store i32 %t5, ptr %t6
  ret void
}

define i32 @main() {
entry:
  %arr = alloca [5 x i32]
  %word = alloca [6 x i8]
  %grid = alloca [2 x [3 x i32]]
  %value = alloca i32
  %p = alloca ptr
  %pp = alloca ptr
  %second = alloca ptr
  %row = alloca ptr
  %left = alloca i32
  %right = alloca i32
  %alias = alloca ptr
  %pt = alloca %struct.Point
  %pp2 = alloca ptr
  %heap = alloca ptr
  %arr.i = alloca i64
  %np = alloca ptr
  store [5 x i32] zeroinitializer, ptr %arr
  %t0 = getelementptr inbounds [5 x i32], ptr %arr, i64 0, i64 0
  store i32 1, ptr %t0
  %t1 = getelementptr inbounds [5 x i32], ptr %arr, i64 0, i64 1
  store i32 2, ptr %t1
  %t2 = getelementptr inbounds [5 x i32], ptr %arr, i64 0, i64 2
  store i32 3, ptr %t2
  store [6 x i8] c"hey\00\00\00", ptr %word
  %t3 = getelementptr inbounds [2 x [3 x i32]], ptr %grid, i64 0, i64 0
  %t4 = getelementptr inbounds [3 x i32], ptr %t3, i64 0, i64 0
  store i32 1, ptr %t4
  %t5 = getelementptr inbounds [3 x i32], ptr %t3, i64 0, i64 1
  store i32 2, ptr %t5
  %t6 = getelementptr inbounds [3 x i32], ptr %t3, i64 0, i64 2
  store i32 3, ptr %t6
  %t7 = getelementptr inbounds [2 x [3 x i32]], ptr %grid, i64 0, i64 1
  %t8 = getelementptr inbounds [3 x i32], ptr %t7, i64 0, i64 0
  store i32 4, ptr %t8
  %t9 = getelementptr inbounds [3 x i32], ptr %t7, i64 0, i64 1
  store i32 5, ptr %t9
  %t10 = getelementptr inbounds [3 x i32], ptr %t7, i64 0, i64 2
  store i32 6, ptr %t10
  %t11 = getelementptr inbounds [13 x i8], ptr @.str, i64 0, i64 0
  %t12 = getelementptr inbounds [5 x i32], ptr %arr, i64 0, i64 0
  %t13 = getelementptr inbounds i32, ptr %t12, i32 0
  %t14 = load i32, ptr %t13
  %t15 = getelementptr inbounds [5 x i32], ptr %arr, i64 0, i64 0
  %t16 = getelementptr inbounds i32, ptr %t15, i32 2
  %t17 = load i32, ptr %t16
  %t18 = getelementptr inbounds [5 x i32], ptr %arr, i64 0, i64 0
  %t19 = getelementptr inbounds i32, ptr %t18, i32 4
  %t20 = load i32, ptr %t19
  %t21 = getelementptr inbounds [6 x i8], ptr %word, i64 0, i64 0
  %t22 = call i32 (ptr, ...) @printf(ptr %t11, i32 %t14, i32 %t17, i32 %t20, ptr %t21)
  %t23 = getelementptr inbounds [7 x i8], ptr @.str.1, i64 0, i64 0
  %t24 = getelementptr inbounds [2 x [3 x i32]], ptr %grid, i64 0, i64 0
  %t25 = getelementptr inbounds [3 x i32], ptr %t24, i32 1
  %t26 = getelementptr inbounds [3 x i32], ptr %t25, i64 0, i64 0
  %t27 = getelementptr inbounds i32, ptr %t26, i32 2
  %t28 = load i32, ptr %t27
  %t29 = getelementptr inbounds [2 x [3 x i32]], ptr %grid, i64 0, i64 0
  %t30 = getelementptr inbounds [3 x i32], ptr %t29, i32 0
  %t31 = getelementptr inbounds [3 x i32], ptr %t30, i64 0, i64 0
  %t32 = getelementptr inbounds i32, ptr %t31, i32 1
  %t33 = load i32, ptr %t32
  %t34 = call i32 (ptr, ...) @printf(ptr %t23, i32 %t28, i32 %t33)
  store i32 7, ptr %value
  store ptr %value, ptr %p
  store ptr %p, ptr %pp
  %t35 = load ptr, ptr %pp
  %t36 = load ptr, ptr %t35
  store i32 8, ptr %t36
  %t37 = getelementptr inbounds [5 x i32], ptr %arr, i64 0, i64 0
  %t38 = getelementptr inbounds i32, ptr %t37, i32 1
  store ptr %t38, ptr %second
  %t39 = getelementptr inbounds [10 x i8], ptr @.str.2, i64 0, i64 0
  %t40 = load i32, ptr %value
  %t41 = load ptr, ptr %second
  %t42 = load i32, ptr %t41
  %t43 = load ptr, ptr %second
  %t44 = getelementptr inbounds [5 x i32], ptr %arr, i64 0, i64 0
  %t45 = ptrtoint ptr %t43 to i64
  %t46 = ptrtoint ptr %t44 to i64
  %t47 = sub i64 %t45, %t46
  %t48 = getelementptr i32, ptr null, i64 1
  %t49 = ptrtoint ptr %t48 to i64
  %t50 = sdiv exact i64 %t47, %t49
  %t51 = trunc i64 %t50 to i32
  %t52 = call i32 (ptr, ...) @printf(ptr %t39, i32 %t40, i32 %t42, i32 %t51)
  %t53 = getelementptr inbounds [2 x [3 x i32]], ptr %grid, i64 0, i64 0
  store ptr %t53, ptr %row
  %t54 = getelementptr inbounds [4 x i8], ptr @.str.3, i64 0, i64 0
  %t55 = load ptr, ptr %row
  %t56 = getelementptr inbounds [3 x i32], ptr %t55, i32 1
  %t57 = getelementptr inbounds [3 x i32], ptr %t56, i64 0, i64 0
  %t58 = getelementptr inbounds i32, ptr %t57, i32 0
  %t59 = load i32, ptr %t58
  %t60 = call i32 (ptr, ...) @printf(ptr %t54, i32 %t59)
  store i32 1, ptr %left
  store i32 2, ptr %right
  call void @global_4swap_RiRi(ptr %left, ptr %right)
  store ptr %left, ptr %alias
  %t61 = load ptr, ptr %alias
  store i32 20, ptr %t61
  %t62 = getelementptr inbounds [7 x i8], ptr @.str.1, i64 0, i64 0
  %t63 = load i32, ptr %left
  %t64 = load i32, ptr %right
  %t65 = call i32 (ptr, ...) @printf(ptr %t62, i32 %t63, i32 %t64)
  %t66 = getelementptr inbounds %struct.Point, ptr %pt, i32 0, i32 0
  store i32 3, ptr %t66
  %t67 = getelementptr inbounds %struct.Point, ptr %pt, i32 0, i32 1
  store i32 4, ptr %t67
  store ptr %pt, ptr %pp2
  %t68 = load ptr, ptr %pp2
  %t69 = getelementptr inbounds %struct.Point, ptr %t68, i32 0, i32 0
  store i32 30, ptr %t69
  %t70 = getelementptr inbounds [7 x i8], ptr @.str.1, i64 0, i64 0
  %t71 = getelementptr inbounds %struct.Point, ptr %pt, i32 0, i32 0
  %t72 = load i32, ptr %t71
  %t73 = getelementptr inbounds %struct.Point, ptr %pt, i32 0, i32 1
  %t74 = load i32, ptr %t73
  %t75 = call i32 (ptr, ...) @printf(ptr %t70, i32 %t72, i32 %t74)
  %t76 = sext i32 3 to i64
  %t77 = getelementptr i32, ptr null, i64 %t76
  %t78 = ptrtoint ptr %t77 to i64
  %t79 = call ptr @malloc(i64 %t78)
  %t80 = getelementptr inbounds i32, ptr %t79, i64 0
  store i32 7, ptr %t80
  %t81 = getelementptr inbounds i32, ptr %t79, i64 1
  store i32 8, ptr %t81
  %t82 = getelementptr inbounds i32, ptr %t79, i64 2
  store i32 9, ptr %t82
  store i64 3, ptr %arr.i
  br label %arr.cond

arr.cond:  ; preds = %entry, %arr.body
  %t83 = load i64, ptr %arr.i
  %t84 = icmp uge i64 %t83, %t76
  br i1 %t84, label %arr.end, label %arr.body

arr.body:  ; preds = %arr.cond
  %t85 = getelementptr inbounds i32, ptr %t79, i64 %t83
  store i32 0, ptr %t85
  %t86 = add i64 %t83, 1
  store i64 %t86, ptr %arr.i
  br label %arr.cond

arr.end:  ; preds = %arr.cond
  store ptr %t79, ptr %heap
  %t87 = getelementptr %struct.Point, ptr null, i64 1
  %t88 = ptrtoint ptr %t87 to i64
  %t89 = call ptr @malloc(i64 %t88)
  %t90 = getelementptr inbounds %struct.Point, ptr %t89, i32 0, i32 0
  store i32 5, ptr %t90
  %t91 = getelementptr inbounds %struct.Point, ptr %t89, i32 0, i32 1
  store i32 6, ptr %t91
  store ptr %t89, ptr %np
  %t92 = getelementptr inbounds [10 x i8], ptr @.str.2, i64 0, i64 0
  %t93 = load ptr, ptr %heap
  %t94 = getelementptr inbounds i32, ptr %t93, i32 2
  %t95 = load i32, ptr %t94
  %t96 = load ptr, ptr %np
  %t97 = getelementptr inbounds %struct.Point, ptr %t96, i32 0, i32 0
  %t98 = load i32, ptr %t97
  %t99 = load ptr, ptr %np
  %t100 = getelementptr inbounds %struct.Point, ptr %t99, i32 0, i32 1
  %t101 = load i32, ptr %t100
  %t102 = call i32 (ptr, ...) @printf(ptr %t92, i32 %t95, i32 %t98, i32 %t101)
  %t103 = load ptr, ptr %heap
  call void @free(ptr %t103)
  %t104 = load ptr, ptr %np
  call void @free(ptr %t104)
  ret i32 0
}

declare ptr @malloc(i64)

declare void @free(ptr)
