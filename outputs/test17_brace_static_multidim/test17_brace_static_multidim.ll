; ModuleID = 'test/test17_brace_static_multidim.ckp'
source_filename = "test/test17_brace_static_multidim.ckp"

%struct.Point = type { i32, i32 }
%class.Counter = type { i32, [2 x [3 x i32]] }

@class_7Counter_5total = global i32 0
@global_9instances = internal global i32 0
@global_5limit = internal global i32 10
@main.calls.0 = internal global i32 0
@main.seen.1 = internal global [4 x i32] zeroinitializer

define internal i32 @global_6helper_i(i32 %x) {
entry:
  %x.addr = alloca i32
  store i32 %x, ptr %x.addr
  %t0 = load i32, ptr %x.addr
  %t1 = mul i32 %t0, 2
  ret i32 %t1
}

define i32 @global_7sumGrid_PA3_ii(ptr %m, i32 %rows) {
entry:
  %m.addr = alloca ptr
  %rows.addr = alloca i32
  %s = alloca i32
  %i = alloca i32
  %j = alloca i32
  store ptr %m, ptr %m.addr
  store i32 %rows, ptr %rows.addr
  store i32 0, ptr %s
  store i32 0, ptr %i
  br label %for.cond

for.cond:  ; preds = %entry, %for.inc
  %t0 = load i32, ptr %i
  %t1 = load i32, ptr %rows.addr
  %t2 = icmp slt i32 %t0, %t1
  br i1 %t2, label %for.body, label %for.end

for.body:  ; preds = %for.cond
  store i32 0, ptr %j
  br label %for.cond.1

for.cond.1:  ; preds = %for.body, %for.inc.1
  %t3 = load i32, ptr %j
  %t4 = icmp slt i32 %t3, 3
  br i1 %t4, label %for.body.1, label %for.end.1

for.body.1:  ; preds = %for.cond.1
  %t5 = load ptr, ptr %m.addr
  %t6 = load i32, ptr %i
  %t7 = getelementptr inbounds [3 x i32], ptr %t5, i32 %t6
  %t8 = getelementptr inbounds [3 x i32], ptr %t7, i64 0, i64 0
  %t9 = load i32, ptr %j
  %t10 = getelementptr inbounds i32, ptr %t8, i32 %t9
  %t11 = load i32, ptr %t10
  %t12 = load i32, ptr %s
  %t13 = add i32 %t12, %t11
  store i32 %t13, ptr %s
  br label %for.inc.1

for.inc.1:  ; preds = %for.body.1
  %t14 = load i32, ptr %j
  %t15 = add i32 %t14, 1
  store i32 %t15, ptr %j
  br label %for.cond.1

for.end.1:  ; preds = %for.cond.1
  br label %for.inc

for.inc:  ; preds = %for.end.1
  %t16 = load i32, ptr %i
  %t17 = add i32 %t16, 1
  store i32 %t17, ptr %i
  br label %for.cond

for.end:  ; preds = %for.cond
  %t18 = load i32, ptr %s
  ret i32 %t18
}

define i32 @main() {
entry:
  %a = alloca i32
  %b = alloca i32
  %arr = alloca [3 x i32]
  %grid = alloca [2 x [3 x i32]]
  %word = alloca [3 x i8]
  %origin = alloca %struct.Point
  %p = alloca %struct.Point
  %q = alloca %struct.Point
  %pp = alloca ptr
  %heap = alloca ptr
  %arr.i = alloca i64
  %np = alloca ptr
  %m = alloca ptr
  %arr.i.1 = alloca i64
  %cube = alloca [2 x [2 x [2 x i32]]]
  %cell = alloca ptr
  %c = alloca %class.Counter
  %cp = alloca ptr
  %total = alloca i32
  %t0 = load i32, ptr @main.calls.0
  %t1 = add i32 %t0, 1
  store i32 %t1, ptr @main.calls.0
  store i32 5, ptr %a
  store i32 0, ptr %b
  %t2 = getelementptr inbounds [3 x i32], ptr %arr, i64 0, i64 0
  store i32 1, ptr %t2
  %t3 = getelementptr inbounds [3 x i32], ptr %arr, i64 0, i64 1
  store i32 2, ptr %t3
  %t4 = getelementptr inbounds [3 x i32], ptr %arr, i64 0, i64 2
  store i32 3, ptr %t4
  %t5 = getelementptr inbounds [2 x [3 x i32]], ptr %grid, i64 0, i64 0
  %t6 = getelementptr inbounds [3 x i32], ptr %t5, i64 0, i64 0
  store i32 1, ptr %t6
  %t7 = getelementptr inbounds [3 x i32], ptr %t5, i64 0, i64 1
  store i32 2, ptr %t7
  %t8 = getelementptr inbounds [3 x i32], ptr %t5, i64 0, i64 2
  store i32 3, ptr %t8
  %t9 = getelementptr inbounds [2 x [3 x i32]], ptr %grid, i64 0, i64 1
  %t10 = getelementptr inbounds [3 x i32], ptr %t9, i64 0, i64 0
  store i32 4, ptr %t10
  %t11 = getelementptr inbounds [3 x i32], ptr %t9, i64 0, i64 1
  store i32 5, ptr %t11
  %t12 = getelementptr inbounds [3 x i32], ptr %t9, i64 0, i64 2
  store i32 6, ptr %t12
  %t13 = getelementptr inbounds [3 x i8], ptr %word, i64 0, i64 0
  store i8 104, ptr %t13
  %t14 = getelementptr inbounds [3 x i8], ptr %word, i64 0, i64 1
  store i8 105, ptr %t14
  %t15 = getelementptr inbounds [3 x i8], ptr %word, i64 0, i64 2
  store i8 0, ptr %t15
  %t16 = getelementptr inbounds %struct.Point, ptr %origin, i32 0, i32 0
  store i32 0, ptr %t16
  %t17 = getelementptr inbounds %struct.Point, ptr %origin, i32 0, i32 1
  store i32 0, ptr %t17
  %t18 = getelementptr inbounds %struct.Point, ptr %p, i32 0, i32 0
  store i32 1, ptr %t18
  %t19 = getelementptr inbounds %struct.Point, ptr %p, i32 0, i32 1
  store i32 2, ptr %t19
  %t20 = getelementptr inbounds %struct.Point, ptr %q, i32 0, i32 0
  store i32 3, ptr %t20
  %t21 = getelementptr inbounds %struct.Point, ptr %q, i32 0, i32 1
  store i32 4, ptr %t21
  store ptr %p, ptr %pp
  %t22 = sext i32 3 to i64
  %t23 = getelementptr i32, ptr null, i64 %t22
  %t24 = ptrtoint ptr %t23 to i64
  %t25 = call ptr @malloc(i64 %t24)
  %t26 = getelementptr inbounds i32, ptr %t25, i64 0
  store i32 7, ptr %t26
  %t27 = getelementptr inbounds i32, ptr %t25, i64 1
  store i32 8, ptr %t27
  %t28 = getelementptr inbounds i32, ptr %t25, i64 2
  store i32 9, ptr %t28
  store i64 3, ptr %arr.i
  br label %arr.cond

arr.cond:  ; preds = %entry, %arr.body
  %t29 = load i64, ptr %arr.i
  %t30 = icmp uge i64 %t29, %t22
  br i1 %t30, label %arr.end, label %arr.body

arr.body:  ; preds = %arr.cond
  %t31 = getelementptr inbounds i32, ptr %t25, i64 %t29
  store i32 0, ptr %t31
  %t32 = add i64 %t29, 1
  store i64 %t32, ptr %arr.i
  br label %arr.cond

arr.end:  ; preds = %arr.cond
  store ptr %t25, ptr %heap
  %t33 = getelementptr %struct.Point, ptr null, i64 1
  %t34 = ptrtoint ptr %t33 to i64
  %t35 = call ptr @malloc(i64 %t34)
  %t36 = getelementptr inbounds %struct.Point, ptr %t35, i32 0, i32 0
  store i32 5, ptr %t36
  %t37 = getelementptr inbounds %struct.Point, ptr %t35, i32 0, i32 1
  store i32 6, ptr %t37
  store ptr %t35, ptr %np
  %t38 = sext i32 2 to i64
  %t39 = getelementptr [3 x i32], ptr null, i64 %t38
  %t40 = ptrtoint ptr %t39 to i64
  %t41 = call ptr @malloc(i64 %t40)
  %t42 = getelementptr inbounds [3 x i32], ptr %t41, i64 0
  %t43 = getelementptr inbounds [3 x i32], ptr %t42, i64 0, i64 0
  store i32 1, ptr %t43
  %t44 = getelementptr inbounds [3 x i32], ptr %t42, i64 0, i64 1
  store i32 2, ptr %t44
  %t45 = getelementptr inbounds [3 x i32], ptr %t42, i64 0, i64 2
  store i32 3, ptr %t45
  %t46 = getelementptr inbounds [3 x i32], ptr %t41, i64 1
  %t47 = getelementptr inbounds [3 x i32], ptr %t46, i64 0, i64 0
  store i32 4, ptr %t47
  %t48 = getelementptr inbounds [3 x i32], ptr %t46, i64 0, i64 1
  store i32 5, ptr %t48
  %t49 = getelementptr inbounds [3 x i32], ptr %t46, i64 0, i64 2
  store i32 6, ptr %t49
  store i64 2, ptr %arr.i.1
  br label %arr.cond.1

arr.cond.1:  ; preds = %arr.end, %arr.body.1
  %t50 = load i64, ptr %arr.i.1
  %t51 = icmp uge i64 %t50, %t38
  br i1 %t51, label %arr.end.1, label %arr.body.1

arr.body.1:  ; preds = %arr.cond.1
  %t52 = getelementptr inbounds [3 x i32], ptr %t41, i64 %t50
  store [3 x i32] zeroinitializer, ptr %t52
  %t53 = add i64 %t50, 1
  store i64 %t53, ptr %arr.i.1
  br label %arr.cond.1

arr.end.1:  ; preds = %arr.cond.1
  store ptr %t41, ptr %m
  %t54 = getelementptr inbounds [2 x [3 x i32]], ptr %grid, i64 0, i64 0
  %t55 = getelementptr inbounds [3 x i32], ptr %t54, i32 1
  %t56 = getelementptr inbounds [3 x i32], ptr %t55, i64 0, i64 0
  %t57 = getelementptr inbounds i32, ptr %t56, i32 2
  %t58 = load i32, ptr %t57
  %t59 = load ptr, ptr %m
  %t60 = getelementptr inbounds [3 x i32], ptr %t59, i32 0
  %t61 = getelementptr inbounds [3 x i32], ptr %t60, i64 0, i64 0
  %t62 = getelementptr inbounds i32, ptr %t61, i32 1
  %t63 = load i32, ptr %t62
  %t64 = add i32 %t58, %t63
  %t65 = getelementptr inbounds [2 x [2 x [2 x i32]]], ptr %cube, i64 0, i64 0
  %t66 = getelementptr inbounds [2 x [2 x i32]], ptr %t65, i32 1
  %t67 = getelementptr inbounds [2 x [2 x i32]], ptr %t66, i64 0, i64 0
  %t68 = getelementptr inbounds [2 x i32], ptr %t67, i32 0
  %t69 = getelementptr inbounds [2 x i32], ptr %t68, i64 0, i64 0
  %t70 = getelementptr inbounds i32, ptr %t69, i32 1
  store i32 %t64, ptr %t70
  %t71 = getelementptr inbounds [2 x [2 x [2 x i32]]], ptr %cube, i64 0, i64 0
  %t72 = getelementptr inbounds [2 x [2 x i32]], ptr %t71, i32 0
  %t73 = getelementptr inbounds [2 x [2 x i32]], ptr %t72, i64 0, i64 0
  %t74 = getelementptr inbounds [2 x i32], ptr %t73, i32 0
  %t75 = getelementptr inbounds [2 x i32], ptr %t74, i64 0, i64 0
  %t76 = getelementptr inbounds i32, ptr %t75, i32 0
  %t77 = load i32, ptr %t76
  %t78 = getelementptr inbounds [2 x [3 x i32]], ptr %grid, i64 0, i64 0
  %t79 = load i32, ptr %a
  %t80 = sub i32 %t79, 4
  %t81 = getelementptr inbounds [3 x i32], ptr %t78, i32 %t80
  %t82 = getelementptr inbounds [3 x i32], ptr %t81, i64 0, i64 0
  %t83 = load i32, ptr %b
  %t84 = getelementptr inbounds i32, ptr %t82, i32 %t83
  %t85 = load i32, ptr %t84
  %t86 = add i32 %t85, %t77
  store i32 %t86, ptr %t84
  %t87 = load ptr, ptr %m
  %t88 = getelementptr inbounds [3 x i32], ptr %t87, i32 1
  %t89 = getelementptr inbounds [3 x i32], ptr %t88, i64 0, i64 0
  %t90 = getelementptr inbounds i32, ptr %t89, i32 2
  %t91 = load i32, ptr %t90
  %t92 = add i32 %t91, 1
  store i32 %t92, ptr %t90
  %t93 = getelementptr inbounds [2 x [3 x i32]], ptr %grid, i64 0, i64 0
  %t94 = getelementptr inbounds [3 x i32], ptr %t93, i32 0
  %t95 = getelementptr inbounds [3 x i32], ptr %t94, i64 0, i64 0
  %t96 = getelementptr inbounds i32, ptr %t95, i32 0
  %t97 = load i32, ptr %t96
  %t98 = sub i32 %t97, 1
  store i32 %t98, ptr %t96
  %t99 = getelementptr inbounds [2 x [3 x i32]], ptr %grid, i64 0, i64 0
  %t100 = getelementptr inbounds [3 x i32], ptr %t99, i32 1
  %t101 = getelementptr inbounds [3 x i32], ptr %t100, i64 0, i64 0
  %t102 = getelementptr inbounds i32, ptr %t101, i32 1
  store ptr %t102, ptr %cell
  call void @class_7Counter_C_v(ptr %c)
  %t103 = getelementptr inbounds %class.Counter, ptr %c, i32 0, i32 1
  %t104 = getelementptr inbounds [2 x [3 x i32]], ptr %t103, i64 0, i64 0
  %t105 = getelementptr inbounds [3 x i32], ptr %t104, i32 1
  %t106 = getelementptr inbounds [3 x i32], ptr %t105, i64 0, i64 0
  %t107 = getelementptr inbounds i32, ptr %t106, i32 2
  store i32 4, ptr %t107
  store ptr %c, ptr %cp
  %t108 = load ptr, ptr %cp
  %t109 = getelementptr inbounds %class.Counter, ptr %t108, i32 0, i32 1
  %t110 = getelementptr inbounds [2 x [3 x i32]], ptr %t109, i64 0, i64 0
  %t111 = getelementptr inbounds [3 x i32], ptr %t110, i32 1
  %t112 = getelementptr inbounds [3 x i32], ptr %t111, i64 0, i64 0
  %t113 = getelementptr inbounds i32, ptr %t112, i32 2
  %t114 = load i32, ptr %t113
  %t115 = mul i32 %t114, 2
  %t116 = load ptr, ptr %cp
  %t117 = getelementptr inbounds %class.Counter, ptr %t116, i32 0, i32 1
  %t118 = getelementptr inbounds [2 x [3 x i32]], ptr %t117, i64 0, i64 0
  %t119 = getelementptr inbounds [3 x i32], ptr %t118, i32 0
  %t120 = getelementptr inbounds [3 x i32], ptr %t119, i64 0, i64 0
  %t121 = getelementptr inbounds i32, ptr %t120, i32 1
  store i32 %t115, ptr %t121
  %t122 = getelementptr inbounds [2 x [3 x i32]], ptr %grid, i64 0, i64 0
  %t123 = call i32 @global_7sumGrid_PA3_ii(ptr %t122, i32 2)
  %t124 = getelementptr inbounds [4 x i32], ptr @main.seen.1, i64 0, i64 0
  %t125 = getelementptr inbounds i32, ptr %t124, i32 0
  %t126 = load i32, ptr %t125
  %t127 = add i32 %t123, %t126
  %t128 = getelementptr inbounds [3 x i32], ptr %arr, i64 0, i64 0
  %t129 = getelementptr inbounds [2 x [3 x i32]], ptr %grid, i64 0, i64 0
  %t130 = getelementptr inbounds [3 x i32], ptr %t129, i32 0
  %t131 = getelementptr inbounds [3 x i32], ptr %t130, i64 0, i64 0
  %t132 = getelementptr inbounds i32, ptr %t131, i32 0
  %t133 = load i32, ptr %t132
  %t134 = getelementptr inbounds i32, ptr %t128, i32 %t133
  %t135 = load i32, ptr %t134
  %t136 = add i32 %t127, %t135
  store i32 %t136, ptr %total
  %t137 = load ptr, ptr %heap
  call void @free(ptr %t137)
  %t138 = load ptr, ptr %m
  call void @free(ptr %t138)
  %t139 = load ptr, ptr %np
  call void @free(ptr %t139)
  %t140 = load i32, ptr %total
  %t141 = load i32, ptr @global_9instances
  %t142 = add i32 %t140, %t141
  %t143 = load i32, ptr @global_5limit
  %t144 = add i32 %t142, %t143
  %t145 = load i32, ptr %a
  %t146 = call i32 @global_6helper_i(i32 %t145)
  %t147 = add i32 %t144, %t146
  %t148 = call i32 @class_7Counter_4next_v()
  %t149 = add i32 %t147, %t148
  %t150 = getelementptr inbounds [3 x i8], ptr %word, i64 0, i64 0
  %t151 = getelementptr inbounds i8, ptr %t150, i32 0
  %t152 = load i8, ptr %t151
  %t153 = sext i8 %t152 to i32
  %t154 = add i32 %t149, %t153
  %t155 = load ptr, ptr %cell
  %t156 = load i32, ptr %t155
  %t157 = add i32 %t154, %t156
  %t158 = load ptr, ptr %pp
  %t159 = getelementptr inbounds %struct.Point, ptr %t158, i32 0, i32 0
  %t160 = load i32, ptr %t159
  %t161 = add i32 %t157, %t160
  %t162 = getelementptr inbounds %struct.Point, ptr %origin, i32 0, i32 1
  %t163 = load i32, ptr %t162
  %t164 = add i32 %t161, %t163
  %t165 = getelementptr inbounds %struct.Point, ptr %q, i32 0, i32 0
  %t166 = load i32, ptr %t165
  %t167 = add i32 %t164, %t166
  ret i32 %t167
}

define i32 @class_7Counter_4next_v() {
entry:
  %t0 = load i32, ptr @class_7Counter_5total
  %t1 = add i32 %t0, 1
  store i32 %t1, ptr @class_7Counter_5total
  %t2 = load i32, ptr @class_7Counter_5total
  ret i32 %t2
}

declare ptr @malloc(i64)

define void @class_7Counter_C_v(ptr %this) {
entry:
  %this.addr = alloca ptr
  store ptr %this, ptr %this.addr
  %t0 = load ptr, ptr %this.addr
  %t1 = getelementptr inbounds %class.Counter, ptr %t0, i32 0, i32 0
  store i32 0, ptr %t1
  ret void
}

declare void @free(ptr)
