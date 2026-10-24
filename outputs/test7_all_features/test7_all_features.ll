; ModuleID = 'test/test7_all_features.ckp'
source_filename = "test/test7_all_features.ckp"

%struct.Point = type { i32, i32 }
%class.Foo = type { i32, i32, i1, i32 }
%class.Bar = type { %class.Foo, i32, i32 }
%closure.262.17 = type {}
%closure.265.22 = type { i32 }
%closure.266.22 = type { ptr }
%closure.267.22 = type { i32 }
%closure.268.22 = type { ptr }
%closure.269.22 = type { i32, ptr }

@.str = private unnamed_addr constant [25 x i8] c"argc = %d, program = %s\0A\00"
@.str.1 = private unnamed_addr constant [19 x i8] c"mode: default run\0A\00"
@.str.2 = private unnamed_addr constant [26 x i8] c"mode: arguments supplied\0A\00"
@.str.3 = private unnamed_addr constant [15 x i8] c"mode: unknown\0A\00"
@.str.4 = private unnamed_addr constant [23 x i8] c"null pointer check ok\0A\00"
@.str.5 = private unnamed_addr constant [17 x i8] c"enter a number: \00"
@.str.6 = private unnamed_addr constant [3 x i8] c"%d\00"
@.str.7 = private unnamed_addr constant [26 x i8] c"you typed %d, doubled %d\0A\00"
@.str.8 = private unnamed_addr constant [12 x i8] c"hello world\00"
@.str.9 = private unnamed_addr constant [37 x i8] c"tab\09here, newline\0A, quote \22, slash \5C\00"
@.str.10 = private unnamed_addr constant [42 x i8] c"not a comment: /* nor this */ // nor this\00"
@.str.11 = private unnamed_addr constant [1 x i8] c"\00"
@.str.12 = private unnamed_addr constant [42 x i8] c"%s squares=%d trace=%d evens=%d tries=%d\0A\00"
@.str.13 = private unnamed_addr constant [38 x i8] c"swapped=%d,%d alias=%d point=(%d,%d)\0A\00"
@.str.14 = private unnamed_addr constant [48 x i8] c"rect area=%d perim=%d heap area=%d heap sum=%d\0A\00"
@.str.15 = private unnamed_addr constant [45 x i8] c"cubed=%d combined=%d gcd=%d fact=%d both=%d\0A\00"
@.str.16 = private unnamed_addr constant [43 x i8] c"mask=%d biggest=%d countdown=%d c=%d t=%d\0A\00"
@.str.17 = private unnamed_addr constant [37 x i8] c"literals %d %d %d %d %f %f %f %f %f\0A\00"
@.str.18 = private unnamed_addr constant [22 x i8] c"chars %c%c%c%c%c%c%c\0A\00"
@.str.19 = private unnamed_addr constant [21 x i8] c"strings %s %s %s %s\0A\00"
@.str.20 = private unnamed_addr constant [24 x i8] c"bools %d %d board %c%c\0A\00"

declare i32 @printf(ptr, ...)

declare i32 @scanf(ptr, ...)

define i32 @global_3add_ii(i32 %a, i32 %b) {
entry:
  %a.addr = alloca i32
  %b.addr = alloca i32
  store i32 %a, ptr %a.addr
  store i32 %b, ptr %b.addr
  %t0 = load i32, ptr %a.addr
  %t1 = load i32, ptr %b.addr
  %t2 = add i32 %t0, %t1
  ret i32 %t2
}

define i32 @global_3gcd_ii(i32 %a, i32 %b) {
entry:
  %a.addr = alloca i32
  %b.addr = alloca i32
  store i32 %a, ptr %a.addr
  store i32 %b, ptr %b.addr
  %t0 = load i32, ptr %b.addr
  %t1 = icmp eq i32 %t0, 0
  br i1 %t1, label %if.then, label %if.end

if.then:  ; preds = %entry
  %t2 = load i32, ptr %a.addr
  ret i32 %t2

if.end:  ; preds = %entry
  %t3 = load i32, ptr %b.addr
  %t4 = load i32, ptr %a.addr
  %t5 = load i32, ptr %b.addr
  %t6 = srem i32 %t4, %t5
  %t7 = call i32 @global_3gcd_ii(i32 %t3, i32 %t6)
  ret i32 %t7
}

define i32 @global_9factorial_i(i32 %n) {
entry:
  %n.addr = alloca i32
  store i32 %n, ptr %n.addr
  %t0 = load i32, ptr %n.addr
  %t1 = icmp sle i32 %t0, 1
  br i1 %t1, label %if.then, label %if.end

if.then:  ; preds = %entry
  ret i32 1

if.end:  ; preds = %entry
  %t2 = load i32, ptr %n.addr
  %t3 = load i32, ptr %n.addr
  %t4 = sub i32 %t3, 1
  %t5 = call i32 @global_9factorial_i(i32 %t4)
  %t6 = mul i32 %t2, %t5
  ret i32 %t6
}

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

define i32 @global_8sumArray_Pii(ptr %data, i32 %count) {
entry:
  %data.addr = alloca ptr
  %count.addr = alloca i32
  %total = alloca i32
  %i = alloca i32
  store ptr %data, ptr %data.addr
  store i32 %count, ptr %count.addr
  store i32 0, ptr %total
  store i32 0, ptr %i
  br label %for.cond

for.cond:  ; preds = %entry, %for.inc
  %t0 = load i32, ptr %i
  %t1 = load i32, ptr %count.addr
  %t2 = icmp slt i32 %t0, %t1
  br i1 %t2, label %for.body, label %for.end

for.body:  ; preds = %for.cond
  %t3 = load ptr, ptr %data.addr
  %t4 = load i32, ptr %i
  %t5 = getelementptr inbounds i32, ptr %t3, i32 %t4
  %t6 = load i32, ptr %t5
  %t7 = load i32, ptr %total
  %t8 = add i32 %t7, %t6
  store i32 %t8, ptr %total
  br label %for.inc

for.inc:  ; preds = %for.body
  %t9 = load i32, ptr %i
  %t10 = add i32 %t9, 1
  store i32 %t10, ptr %i
  br label %for.cond

for.end:  ; preds = %for.cond
  %t11 = load i32, ptr %total
  ret i32 %t11
}

define i32 @global_5addOp_ii(i32 %a, i32 %b) {
entry:
  %a.addr = alloca i32
  %b.addr = alloca i32
  store i32 %a, ptr %a.addr
  store i32 %b, ptr %b.addr
  %t0 = load i32, ptr %a.addr
  %t1 = load i32, ptr %b.addr
  %t2 = add i32 %t0, %t1
  ret i32 %t2
}

define i32 @global_5mulOp_ii(i32 %a, i32 %b) {
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

define i32 @main(i32 %argc, ptr %argv) {
entry:
  %argc.addr = alloca i32
  %argv.addr = alloca ptr
  %mode = alloca i32
  %a = alloca i32
  %b = alloca i32
  %c = alloca i32
  %t = alloca i1
  %and.result = alloca i1
  %or.result = alloca i1
  %biggest = alloca i32
  %cond.result = alloca ptr
  %mask = alloca i32
  %sum = alloca i32
  %i = alloca i32
  %countdown = alloca i32
  %evens = alloca i32
  %tries = alloca i32
  %values = alloca [8 x i32]
  %squares = alloca i32
  %label = alloca [8 x i8]
  %grid = alloca [3 x [3 x i32]]
  %board = alloca [2 x [2 x i8]]
  %row = alloca i32
  %col = alloca i32
  %trace = alloca i32
  %value = alloca i32
  %ptr = alloca ptr
  %read = alloca i32
  %alias = alloca ptr
  %left = alloca i32
  %right = alloca i32
  %origin = alloca %struct.Point
  %pointPtr = alloca ptr
  %bar = alloca %class.Bar
  %rectArea = alloca i32
  %rectPerim = alloca i32
  %barPtr = alloca ptr
  %heapArea = alloca i32
  %heapData = alloca ptr
  %heapSum = alloca i32
  %cube = alloca %closure.262.17
  %byValue = alloca %closure.265.22
  %byRef = alloca %closure.266.22
  %allValue = alloca %closure.267.22
  %allRef = alloca %closure.268.22
  %mixed = alloca %closure.269.22
  %cubed = alloca i32
  %binaryOp = alloca ptr
  %combined = alloca i32
  %g = alloca i32
  %f = alloca i32
  %both = alloca i32
  %typed = alloca i32
  %decNum = alloca i32
  %hexNum = alloca i32
  %octNum = alloca i32
  %binNum = alloca i32
  %pi = alloca float
  %tiny = alloca float
  %half = alloca double
  %two = alloca double
  %huge = alloca double
  %letter = alloca i8
  %newline = alloca i8
  %tab = alloca i8
  %quote = alloca i8
  %slash = alloca i8
  %hexChar = alloca i8
  %nul = alloca i8
  %greeting = alloca ptr
  %escaped = alloca ptr
  %tricky = alloca ptr
  %empty = alloca ptr
  %yes = alloca i1
  %no = alloca i1
  store i32 %argc, ptr %argc.addr
  store ptr %argv, ptr %argv.addr
  store i32 1, ptr %mode
  %t0 = load i32, ptr %argc.addr
  %t1 = icmp sgt i32 %t0, 1
  br i1 %t1, label %if.then, label %if.end

if.then:  ; preds = %entry
  store i32 2, ptr %mode
  br label %if.end

if.end:  ; preds = %entry, %if.then
  %t2 = getelementptr inbounds [25 x i8], ptr @.str, i64 0, i64 0
  %t3 = load i32, ptr %argc.addr
  %t4 = load ptr, ptr %argv.addr
  %t5 = getelementptr inbounds ptr, ptr %t4, i32 0
  %t6 = load ptr, ptr %t5
  %t7 = call i32 (ptr, ...) @printf(ptr %t2, i32 %t3, ptr %t6)
  store i32 10, ptr %a
  store i32 3, ptr %b
  %t8 = load i32, ptr %a
  %t9 = load i32, ptr %b
  %t10 = add i32 %t8, %t9
  store i32 %t10, ptr %c
  %t11 = load i32, ptr %a
  %t12 = load i32, ptr %b
  %t13 = sub i32 %t11, %t12
  store i32 %t13, ptr %c
  %t14 = load i32, ptr %a
  %t15 = load i32, ptr %b
  %t16 = mul i32 %t14, %t15
  store i32 %t16, ptr %c
  %t17 = load i32, ptr %a
  %t18 = load i32, ptr %b
  %t19 = sdiv i32 %t17, %t18
  store i32 %t19, ptr %c
  %t20 = load i32, ptr %a
  %t21 = load i32, ptr %b
  %t22 = srem i32 %t20, %t21
  store i32 %t22, ptr %c
  %t23 = load i32, ptr %a
  %t24 = sub i32 0, %t23
  store i32 %t24, ptr %c
  %t25 = load i32, ptr %a
  store i32 %t25, ptr %c
  %t26 = load i32, ptr %a
  %t27 = add i32 %t26, 1
  store i32 %t27, ptr %a
  %t28 = load i32, ptr %a
  %t29 = sub i32 %t28, 1
  store i32 %t29, ptr %a
  %t30 = load i32, ptr %a
  %t31 = add i32 %t30, 1
  store i32 %t31, ptr %a
  %t32 = load i32, ptr %a
  %t33 = sub i32 %t32, 1
  store i32 %t33, ptr %a
  %t34 = load i32, ptr %a
  %t35 = load i32, ptr %c
  %t36 = add i32 %t35, %t34
  store i32 %t36, ptr %c
  %t37 = load i32, ptr %a
  %t38 = load i32, ptr %c
  %t39 = sub i32 %t38, %t37
  store i32 %t39, ptr %c
  %t40 = load i32, ptr %a
  %t41 = load i32, ptr %c
  %t42 = mul i32 %t41, %t40
  store i32 %t42, ptr %c
  %t43 = load i32, ptr %a
  %t44 = load i32, ptr %c
  %t45 = sdiv i32 %t44, %t43
  store i32 %t45, ptr %c
  %t46 = load i32, ptr %a
  %t47 = load i32, ptr %c
  %t48 = srem i32 %t47, %t46
  store i32 %t48, ptr %c
  %t49 = load i32, ptr %a
  %t50 = load i32, ptr %b
  %t51 = icmp eq i32 %t49, %t50
  store i1 %t51, ptr %t
  %t52 = load i32, ptr %a
  %t53 = load i32, ptr %b
  %t54 = icmp ne i32 %t52, %t53
  store i1 %t54, ptr %t
  %t55 = load i32, ptr %a
  %t56 = load i32, ptr %b
  %t57 = icmp slt i32 %t55, %t56
  store i1 %t57, ptr %t
  %t58 = load i32, ptr %a
  %t59 = load i32, ptr %b
  %t60 = icmp sgt i32 %t58, %t59
  store i1 %t60, ptr %t
  %t61 = load i32, ptr %a
  %t62 = load i32, ptr %b
  %t63 = icmp sle i32 %t61, %t62
  store i1 %t63, ptr %t
  %t64 = load i32, ptr %a
  %t65 = load i32, ptr %b
  %t66 = icmp sge i32 %t64, %t65
  store i1 %t66, ptr %t
  %t67 = load i32, ptr %a
  %t68 = icmp ne i32 %t67, 0
  br i1 %t68, label %land.rhs, label %land.short

land.short:  ; preds = %if.end
  store i1 false, ptr %and.result
  br label %land.end

land.rhs:  ; preds = %if.end
  %t69 = load i32, ptr %b
  %t70 = icmp ne i32 %t69, 0
  store i1 %t70, ptr %and.result
  br label %land.end

land.end:  ; preds = %land.short, %land.rhs
  %t71 = load i1, ptr %and.result
  store i1 %t71, ptr %t
  %t72 = load i32, ptr %a
  %t73 = icmp ne i32 %t72, 0
  br i1 %t73, label %lor.short, label %lor.rhs

lor.short:  ; preds = %land.end
  store i1 true, ptr %or.result
  br label %lor.end

lor.rhs:  ; preds = %land.end
  %t74 = load i32, ptr %b
  %t75 = icmp ne i32 %t74, 0
  store i1 %t75, ptr %or.result
  br label %lor.end

lor.end:  ; preds = %lor.short, %lor.rhs
  %t76 = load i1, ptr %or.result
  store i1 %t76, ptr %t
  %t77 = load i1, ptr %t
  %t78 = xor i1 %t77, true
  store i1 %t78, ptr %t
  store i1 true, ptr %t
  store i1 false, ptr %t
  %t79 = load i32, ptr %a
  %t80 = load i32, ptr %b
  %t81 = icmp sgt i32 %t79, %t80
  br i1 %t81, label %cond.true, label %cond.false

cond.true:  ; preds = %lor.end
  store ptr %a, ptr %cond.result
  br label %cond.end

cond.false:  ; preds = %lor.end
  store ptr %b, ptr %cond.result
  br label %cond.end

cond.end:  ; preds = %cond.true, %cond.false
  %t82 = load ptr, ptr %cond.result
  %t83 = load i32, ptr %t82
  store i32 %t83, ptr %biggest
  %t84 = load i32, ptr %a
  %t85 = load i32, ptr %b
  %t86 = and i32 %t84, %t85
  store i32 %t86, ptr %mask
  %t87 = load i32, ptr %a
  %t88 = load i32, ptr %b
  %t89 = or i32 %t87, %t88
  store i32 %t89, ptr %mask
  %t90 = load i32, ptr %a
  %t91 = load i32, ptr %b
  %t92 = xor i32 %t90, %t91
  store i32 %t92, ptr %mask
  %t93 = load i32, ptr %a
  %t94 = xor i32 %t93, -1
  store i32 %t94, ptr %mask
  %t95 = load i32, ptr %a
  %t96 = shl i32 %t95, 2
  store i32 %t96, ptr %mask
  %t97 = load i32, ptr %a
  %t98 = ashr i32 %t97, 2
  store i32 %t98, ptr %mask
  %t99 = load i32, ptr %biggest
  %t100 = icmp slt i32 %t99, 0
  br i1 %t100, label %if.then.1, label %if.else

if.then.1:  ; preds = %cond.end
  store i32 0, ptr %c
  br label %if.end.1

if.else:  ; preds = %cond.end
  %t101 = load i32, ptr %biggest
  %t102 = icmp eq i32 %t101, 0
  br i1 %t102, label %if.then.2, label %if.else.1

if.then.2:  ; preds = %if.else
  store i32 1, ptr %c
  br label %if.end.2

if.else.1:  ; preds = %if.else
  store i32 2, ptr %c
  br label %if.end.2

if.end.2:  ; preds = %if.then.2, %if.else.1
  br label %if.end.1

if.end.1:  ; preds = %if.then.1, %if.end.2
  store i32 0, ptr %sum
  store i32 0, ptr %i
  br label %for.cond

for.cond:  ; preds = %if.end.1, %for.inc
  %t103 = load i32, ptr %i
  %t104 = icmp slt i32 %t103, 10
  br i1 %t104, label %for.body, label %for.end

for.body:  ; preds = %for.cond
  %t105 = load i32, ptr %i
  %t106 = load i32, ptr %sum
  %t107 = add i32 %t106, %t105
  store i32 %t107, ptr %sum
  br label %for.inc

for.inc:  ; preds = %for.body
  %t108 = load i32, ptr %i
  %t109 = add i32 %t108, 1
  store i32 %t109, ptr %i
  br label %for.cond

for.end:  ; preds = %for.cond
  br label %while.cond

while.cond:  ; preds = %for.end, %while.body
  %t110 = load i32, ptr %sum
  %t111 = icmp sgt i32 %t110, 20
  br i1 %t111, label %while.body, label %while.end

while.body:  ; preds = %while.cond
  %t112 = load i32, ptr %sum
  %t113 = sub i32 %t112, 1
  store i32 %t113, ptr %sum
  br label %while.cond

while.end:  ; preds = %while.cond
  store i32 3, ptr %countdown
  br label %do.body

do.body:  ; preds = %while.end, %do.cond
  %t114 = load i32, ptr %countdown
  %t115 = sub i32 %t114, 1
  store i32 %t115, ptr %countdown
  br label %do.cond

do.cond:  ; preds = %do.body
  %t116 = load i32, ptr %countdown
  %t117 = icmp sgt i32 %t116, 0
  br i1 %t117, label %do.body, label %do.end

do.end:  ; preds = %do.cond
  %t118 = load i32, ptr %mode
  switch i32 %t118, label %sw.default [
    i32 1, label %sw.case
    i32 2, label %sw.case.1
  ]

sw.case:  ; preds = %do.end
  %t119 = getelementptr inbounds [19 x i8], ptr @.str.1, i64 0, i64 0
  %t120 = call i32 (ptr, ...) @printf(ptr %t119)
  br label %sw.end

sw.case.1:  ; preds = %do.end
  %t121 = getelementptr inbounds [26 x i8], ptr @.str.2, i64 0, i64 0
  %t122 = call i32 (ptr, ...) @printf(ptr %t121)
  br label %sw.end

sw.default:  ; preds = %do.end
  %t123 = getelementptr inbounds [15 x i8], ptr @.str.3, i64 0, i64 0
  %t124 = call i32 (ptr, ...) @printf(ptr %t123)
  br label %sw.end

sw.end:  ; preds = %sw.case, %sw.case.1, %sw.default
  store i32 0, ptr %evens
  store i32 0, ptr %i
  br label %for.cond.1

for.cond.1:  ; preds = %sw.end, %for.inc.1
  %t125 = load i32, ptr %i
  %t126 = icmp slt i32 %t125, 20
  br i1 %t126, label %for.body.1, label %for.end.1

for.body.1:  ; preds = %for.cond.1
  %t127 = load i32, ptr %i
  %t128 = srem i32 %t127, 2
  %t129 = icmp ne i32 %t128, 0
  br i1 %t129, label %if.then.3, label %if.end.3

if.then.3:  ; preds = %for.body.1
  br label %for.inc.1

if.end.3:  ; preds = %for.body.1
  %t130 = load i32, ptr %i
  %t131 = icmp sgt i32 %t130, 12
  br i1 %t131, label %if.then.4, label %if.end.4

if.then.4:  ; preds = %if.end.3
  br label %for.end.1

if.end.4:  ; preds = %if.end.3
  %t132 = load i32, ptr %i
  %t133 = load i32, ptr %evens
  %t134 = add i32 %t133, %t132
  store i32 %t134, ptr %evens
  br label %for.inc.1

for.inc.1:  ; preds = %if.then.3, %if.end.4
  %t135 = load i32, ptr %i
  %t136 = add i32 %t135, 1
  store i32 %t136, ptr %i
  br label %for.cond.1

for.end.1:  ; preds = %for.cond.1, %if.then.4
  store i32 0, ptr %tries
  br label %retry

retry:  ; preds = %for.end.1, %if.then.5
  %t137 = load i32, ptr %tries
  %t138 = add i32 %t137, 1
  store i32 %t138, ptr %tries
  %t139 = load i32, ptr %tries
  %t140 = icmp slt i32 %t139, 3
  br i1 %t140, label %if.then.5, label %if.end.5

if.then.5:  ; preds = %retry
  br label %retry

if.end.5:  ; preds = %retry
  store i32 0, ptr %i
  br label %for.cond.2

for.cond.2:  ; preds = %if.end.5, %for.inc.2
  %t141 = load i32, ptr %i
  %t142 = icmp slt i32 %t141, 8
  br i1 %t142, label %for.body.2, label %for.end.2

for.body.2:  ; preds = %for.cond.2
  %t143 = load i32, ptr %i
  %t144 = add i32 %t143, 1
  %t145 = load i32, ptr %i
  %t146 = add i32 %t145, 1
  %t147 = mul i32 %t144, %t146
  %t148 = getelementptr inbounds [8 x i32], ptr %values, i64 0, i64 0
  %t149 = load i32, ptr %i
  %t150 = getelementptr inbounds i32, ptr %t148, i32 %t149
  store i32 %t147, ptr %t150
  br label %for.inc.2

for.inc.2:  ; preds = %for.body.2
  %t151 = load i32, ptr %i
  %t152 = add i32 %t151, 1
  store i32 %t152, ptr %i
  br label %for.cond.2

for.end.2:  ; preds = %for.cond.2
  %t153 = getelementptr inbounds [8 x i32], ptr %values, i64 0, i64 0
  %t154 = call i32 @global_8sumArray_Pii(ptr %t153, i32 8)
  store i32 %t154, ptr %squares
  %t155 = getelementptr inbounds [8 x i8], ptr %label, i64 0, i64 0
  %t156 = getelementptr inbounds i8, ptr %t155, i32 0
  store i8 82, ptr %t156
  %t157 = getelementptr inbounds [8 x i8], ptr %label, i64 0, i64 0
  %t158 = getelementptr inbounds i8, ptr %t157, i32 1
  store i8 69, ptr %t158
  %t159 = getelementptr inbounds [8 x i8], ptr %label, i64 0, i64 0
  %t160 = getelementptr inbounds i8, ptr %t159, i32 2
  store i8 83, ptr %t160
  %t161 = getelementptr inbounds [8 x i8], ptr %label, i64 0, i64 0
  %t162 = getelementptr inbounds i8, ptr %t161, i32 3
  store i8 85, ptr %t162
  %t163 = getelementptr inbounds [8 x i8], ptr %label, i64 0, i64 0
  %t164 = getelementptr inbounds i8, ptr %t163, i32 4
  store i8 76, ptr %t164
  %t165 = getelementptr inbounds [8 x i8], ptr %label, i64 0, i64 0
  %t166 = getelementptr inbounds i8, ptr %t165, i32 5
  store i8 84, ptr %t166
  %t167 = getelementptr inbounds [8 x i8], ptr %label, i64 0, i64 0
  %t168 = getelementptr inbounds i8, ptr %t167, i32 6
  store i8 58, ptr %t168
  %t169 = getelementptr inbounds [8 x i8], ptr %label, i64 0, i64 0
  %t170 = getelementptr inbounds i8, ptr %t169, i32 7
  store i8 0, ptr %t170
  store i32 0, ptr %row
  br label %for.cond.3

for.cond.3:  ; preds = %for.end.2, %for.inc.3
  %t171 = load i32, ptr %row
  %t172 = icmp slt i32 %t171, 3
  br i1 %t172, label %for.body.3, label %for.end.3

for.body.3:  ; preds = %for.cond.3
  store i32 0, ptr %col
  br label %for.cond.4

for.cond.4:  ; preds = %for.body.3, %for.inc.4
  %t173 = load i32, ptr %col
  %t174 = icmp slt i32 %t173, 3
  br i1 %t174, label %for.body.4, label %for.end.4

for.body.4:  ; preds = %for.cond.4
  %t175 = load i32, ptr %row
  %t176 = mul i32 %t175, 3
  %t177 = load i32, ptr %col
  %t178 = add i32 %t176, %t177
  %t179 = getelementptr inbounds [3 x [3 x i32]], ptr %grid, i64 0, i64 0
  %t180 = load i32, ptr %row
  %t181 = getelementptr inbounds [3 x i32], ptr %t179, i32 %t180
  %t182 = getelementptr inbounds [3 x i32], ptr %t181, i64 0, i64 0
  %t183 = load i32, ptr %col
  %t184 = getelementptr inbounds i32, ptr %t182, i32 %t183
  store i32 %t178, ptr %t184
  br label %for.inc.4

for.inc.4:  ; preds = %for.body.4
  %t185 = load i32, ptr %col
  %t186 = add i32 %t185, 1
  store i32 %t186, ptr %col
  br label %for.cond.4

for.end.4:  ; preds = %for.cond.4
  br label %for.inc.3

for.inc.3:  ; preds = %for.end.4
  %t187 = load i32, ptr %row
  %t188 = add i32 %t187, 1
  store i32 %t188, ptr %row
  br label %for.cond.3

for.end.3:  ; preds = %for.cond.3
  %t189 = getelementptr inbounds [2 x [2 x i8]], ptr %board, i64 0, i64 0
  %t190 = getelementptr inbounds [2 x i8], ptr %t189, i32 0
  %t191 = getelementptr inbounds [2 x i8], ptr %t190, i64 0, i64 0
  %t192 = getelementptr inbounds i8, ptr %t191, i32 0
  store i8 120, ptr %t192
  %t193 = getelementptr inbounds [2 x [2 x i8]], ptr %board, i64 0, i64 0
  %t194 = getelementptr inbounds [2 x i8], ptr %t193, i32 1
  %t195 = getelementptr inbounds [2 x i8], ptr %t194, i64 0, i64 0
  %t196 = getelementptr inbounds i8, ptr %t195, i32 1
  store i8 111, ptr %t196
  %t197 = getelementptr inbounds [3 x [3 x i32]], ptr %grid, i64 0, i64 0
  %t198 = getelementptr inbounds [3 x i32], ptr %t197, i32 0
  %t199 = getelementptr inbounds [3 x i32], ptr %t198, i64 0, i64 0
  %t200 = getelementptr inbounds i32, ptr %t199, i32 0
  %t201 = load i32, ptr %t200
  %t202 = getelementptr inbounds [3 x [3 x i32]], ptr %grid, i64 0, i64 0
  %t203 = getelementptr inbounds [3 x i32], ptr %t202, i32 1
  %t204 = getelementptr inbounds [3 x i32], ptr %t203, i64 0, i64 0
  %t205 = getelementptr inbounds i32, ptr %t204, i32 1
  %t206 = load i32, ptr %t205
  %t207 = add i32 %t201, %t206
  %t208 = getelementptr inbounds [3 x [3 x i32]], ptr %grid, i64 0, i64 0
  %t209 = getelementptr inbounds [3 x i32], ptr %t208, i32 2
  %t210 = getelementptr inbounds [3 x i32], ptr %t209, i64 0, i64 0
  %t211 = getelementptr inbounds i32, ptr %t210, i32 2
  %t212 = load i32, ptr %t211
  %t213 = add i32 %t207, %t212
  store i32 %t213, ptr %trace
  store i32 99, ptr %value
  store ptr %value, ptr %ptr
  %t214 = load ptr, ptr %ptr
  %t215 = load i32, ptr %t214
  store i32 %t215, ptr %read
  store ptr null, ptr %ptr
  %t216 = load ptr, ptr %ptr
  %t217 = icmp eq ptr %t216, null
  br i1 %t217, label %if.then.6, label %if.end.6

if.then.6:  ; preds = %for.end.3
  %t218 = getelementptr inbounds [23 x i8], ptr @.str.4, i64 0, i64 0
  %t219 = call i32 (ptr, ...) @printf(ptr %t218)
  br label %if.end.6

if.end.6:  ; preds = %for.end.3, %if.then.6
  store ptr %value, ptr %alias
  %t220 = load i32, ptr %read
  %t221 = load ptr, ptr %alias
  store i32 %t220, ptr %t221
  store i32 11, ptr %left
  store i32 22, ptr %right
  call void @global_4swap_RiRi(ptr %left, ptr %right)
  %t222 = getelementptr inbounds %struct.Point, ptr %origin, i32 0, i32 0
  store i32 0, ptr %t222
  %t223 = getelementptr inbounds %struct.Point, ptr %origin, i32 0, i32 1
  store i32 0, ptr %t223
  store ptr %origin, ptr %pointPtr
  %t224 = load ptr, ptr %pointPtr
  %t225 = getelementptr inbounds %struct.Point, ptr %t224, i32 0, i32 0
  store i32 3, ptr %t225
  %t226 = load ptr, ptr %pointPtr
  %t227 = getelementptr inbounds %struct.Point, ptr %t226, i32 0, i32 1
  store i32 4, ptr %t227
  %t228 = getelementptr inbounds %class.Bar, ptr %bar, i32 0, i32 0
  call void @class_3Foo_5setId_i(ptr %t228, i32 1)
  %t229 = getelementptr inbounds %class.Bar, ptr %bar, i32 0, i32 0
  call void @class_3Foo_9setFilled_b(ptr %t229, i1 true)
  call void @class_3Bar_4init_ii(ptr %bar, i32 6, i32 7)
  %t230 = call i32 @class_3Bar_4area_v(ptr %bar)
  store i32 %t230, ptr %rectArea
  %t231 = call i32 @class_3Bar_9perimeter_v(ptr %bar)
  store i32 %t231, ptr %rectPerim
  %t232 = getelementptr %class.Bar, ptr null, i64 1
  %t233 = ptrtoint ptr %t232 to i64
  %t234 = call ptr @malloc(i64 %t233)
  store ptr %t234, ptr %barPtr
  %t235 = load ptr, ptr %barPtr
  %t236 = getelementptr inbounds %class.Bar, ptr %t235, i32 0, i32 0
  call void @class_3Foo_5setId_i(ptr %t236, i32 2)
  %t237 = load ptr, ptr %barPtr
  call void @class_3Bar_4init_ii(ptr %t237, i32 10, i32 20)
  %t238 = load ptr, ptr %barPtr
  %t239 = call i32 @class_3Bar_4area_v(ptr %t238)
  store i32 %t239, ptr %heapArea
  %t240 = load ptr, ptr %barPtr
  call void @free(ptr %t240)
  %t241 = sext i32 5 to i64
  %t242 = getelementptr i32, ptr null, i64 %t241
  %t243 = ptrtoint ptr %t242 to i64
  %t244 = call ptr @malloc(i64 %t243)
  store ptr %t244, ptr %heapData
  store i32 0, ptr %i
  br label %for.cond.5

for.cond.5:  ; preds = %if.end.6, %for.inc.5
  %t245 = load i32, ptr %i
  %t246 = icmp slt i32 %t245, 5
  br i1 %t246, label %for.body.5, label %for.end.5

for.body.5:  ; preds = %for.cond.5
  %t247 = load i32, ptr %i
  %t248 = mul i32 %t247, 3
  %t249 = load ptr, ptr %heapData
  %t250 = load i32, ptr %i
  %t251 = getelementptr inbounds i32, ptr %t249, i32 %t250
  store i32 %t248, ptr %t251
  br label %for.inc.5

for.inc.5:  ; preds = %for.body.5
  %t252 = load i32, ptr %i
  %t253 = add i32 %t252, 1
  store i32 %t253, ptr %i
  br label %for.cond.5

for.end.5:  ; preds = %for.cond.5
  %t254 = load ptr, ptr %heapData
  %t255 = call i32 @global_8sumArray_Pii(ptr %t254, i32 5)
  store i32 %t255, ptr %heapSum
  %t256 = load ptr, ptr %heapData
  call void @free(ptr %t256)
  %t257 = getelementptr inbounds %closure.265.22, ptr %byValue, i32 0, i32 0
  %t258 = load i32, ptr %squares
  store i32 %t258, ptr %t257
  %t259 = getelementptr inbounds %closure.266.22, ptr %byRef, i32 0, i32 0
  store ptr %squares, ptr %t259
  %t260 = getelementptr inbounds %closure.267.22, ptr %allValue, i32 0, i32 0
  %t261 = load i32, ptr %squares
  store i32 %t261, ptr %t260
  %t262 = getelementptr inbounds %closure.268.22, ptr %allRef, i32 0, i32 0
  store ptr %squares, ptr %t262
  %t263 = getelementptr inbounds %closure.269.22, ptr %mixed, i32 0, i32 0
  %t264 = load i32, ptr %squares
  store i32 %t264, ptr %t263
  %t265 = getelementptr inbounds %closure.269.22, ptr %mixed, i32 0, i32 1
  store ptr %trace, ptr %t265
  %t266 = call i32 @__lambda_262_17(ptr %cube, i32 4)
  store i32 %t266, ptr %cubed
  store ptr @global_5addOp_ii, ptr %binaryOp
  %t267 = load ptr, ptr %binaryOp
  %t268 = load i32, ptr %left
  %t269 = load i32, ptr %right
  %t270 = call i32 %t267(i32 %t268, i32 %t269)
  store i32 %t270, ptr %combined
  store ptr @global_5mulOp_ii, ptr %binaryOp
  %t271 = load i32, ptr %combined
  %t272 = load ptr, ptr %binaryOp
  %t273 = call i32 %t272(i32 2, i32 5)
  %t274 = add i32 %t271, %t273
  store i32 %t274, ptr %combined
  %t275 = call i32 @global_3gcd_ii(i32 48, i32 18)
  store i32 %t275, ptr %g
  %t276 = call i32 @global_9factorial_i(i32 6)
  store i32 %t276, ptr %f
  %t277 = load i32, ptr %g
  %t278 = load i32, ptr %f
  %t279 = call i32 @global_3add_ii(i32 %t277, i32 %t278)
  store i32 %t279, ptr %both
  store i32 0, ptr %typed
  %t280 = getelementptr inbounds [17 x i8], ptr @.str.5, i64 0, i64 0
  %t281 = call i32 (ptr, ...) @printf(ptr %t280)
  %t282 = getelementptr inbounds [3 x i8], ptr @.str.6, i64 0, i64 0
  %t283 = call i32 (ptr, ...) @scanf(ptr %t282, ptr %typed)
  %t284 = getelementptr inbounds [26 x i8], ptr @.str.7, i64 0, i64 0
  %t285 = load i32, ptr %typed
  %t286 = load i32, ptr %typed
  %t287 = mul i32 %t286, 2
  %t288 = call i32 (ptr, ...) @printf(ptr %t284, i32 %t285, i32 %t287)
  store i32 42, ptr %decNum
  store i32 31, ptr %hexNum
  store i32 493, ptr %octNum
  store i32 10, ptr %binNum
  %t289 = fptrunc double 0x40091EB851EB851F to float
  store float %t289, ptr %pi
  %t290 = fptrunc double 0x3F589374BC6A7EFA to float
  store float %t290, ptr %tiny
  store double 0x3FE0000000000000, ptr %half
  store double 0x4000000000000000, ptr %two
  store double 0x4202A05F20000000, ptr %huge
  store i8 97, ptr %letter
  store i8 10, ptr %newline
  store i8 9, ptr %tab
  store i8 39, ptr %quote
  store i8 92, ptr %slash
  store i8 65, ptr %hexChar
  store i8 0, ptr %nul
  %t291 = getelementptr inbounds [12 x i8], ptr @.str.8, i64 0, i64 0
  store ptr %t291, ptr %greeting
  %t292 = getelementptr inbounds [37 x i8], ptr @.str.9, i64 0, i64 0
  store ptr %t292, ptr %escaped
  %t293 = getelementptr inbounds [42 x i8], ptr @.str.10, i64 0, i64 0
  store ptr %t293, ptr %tricky
  %t294 = getelementptr inbounds [1 x i8], ptr @.str.11, i64 0, i64 0
  store ptr %t294, ptr %empty
  store i1 true, ptr %yes
  store i1 false, ptr %no
  %t295 = getelementptr inbounds [42 x i8], ptr @.str.12, i64 0, i64 0
  %t296 = getelementptr inbounds [8 x i8], ptr %label, i64 0, i64 0
  %t297 = load i32, ptr %squares
  %t298 = load i32, ptr %trace
  %t299 = load i32, ptr %evens
  %t300 = load i32, ptr %tries
  %t301 = call i32 (ptr, ...) @printf(ptr %t295, ptr %t296, i32 %t297, i32 %t298, i32 %t299, i32 %t300)
  %t302 = getelementptr inbounds [38 x i8], ptr @.str.13, i64 0, i64 0
  %t303 = load i32, ptr %left
  %t304 = load i32, ptr %right
  %t305 = load ptr, ptr %alias
  %t306 = load i32, ptr %t305
  %t307 = getelementptr inbounds %struct.Point, ptr %origin, i32 0, i32 0
  %t308 = load i32, ptr %t307
  %t309 = getelementptr inbounds %struct.Point, ptr %origin, i32 0, i32 1
  %t310 = load i32, ptr %t309
  %t311 = call i32 (ptr, ...) @printf(ptr %t302, i32 %t303, i32 %t304, i32 %t306, i32 %t308, i32 %t310)
  %t312 = getelementptr inbounds [48 x i8], ptr @.str.14, i64 0, i64 0
  %t313 = load i32, ptr %rectArea
  %t314 = load i32, ptr %rectPerim
  %t315 = load i32, ptr %heapArea
  %t316 = load i32, ptr %heapSum
  %t317 = call i32 (ptr, ...) @printf(ptr %t312, i32 %t313, i32 %t314, i32 %t315, i32 %t316)
  %t318 = getelementptr inbounds [45 x i8], ptr @.str.15, i64 0, i64 0
  %t319 = load i32, ptr %cubed
  %t320 = load i32, ptr %combined
  %t321 = load i32, ptr %g
  %t322 = load i32, ptr %f
  %t323 = load i32, ptr %both
  %t324 = call i32 (ptr, ...) @printf(ptr %t318, i32 %t319, i32 %t320, i32 %t321, i32 %t322, i32 %t323)
  %t325 = getelementptr inbounds [43 x i8], ptr @.str.16, i64 0, i64 0
  %t326 = load i32, ptr %mask
  %t327 = load i32, ptr %biggest
  %t328 = load i32, ptr %countdown
  %t329 = load i32, ptr %c
  %t330 = load i1, ptr %t
  %t331 = zext i1 %t330 to i32
  %t332 = call i32 (ptr, ...) @printf(ptr %t325, i32 %t326, i32 %t327, i32 %t328, i32 %t329, i32 %t331)
  %t333 = getelementptr inbounds [37 x i8], ptr @.str.17, i64 0, i64 0
  %t334 = load i32, ptr %decNum
  %t335 = load i32, ptr %hexNum
  %t336 = load i32, ptr %octNum
  %t337 = load i32, ptr %binNum
  %t338 = load float, ptr %pi
  %t339 = fpext float %t338 to double
  %t340 = load float, ptr %tiny
  %t341 = fpext float %t340 to double
  %t342 = load double, ptr %half
  %t343 = load double, ptr %two
  %t344 = load double, ptr %huge
  %t345 = call i32 (ptr, ...) @printf(ptr %t333, i32 %t334, i32 %t335, i32 %t336, i32 %t337, double %t339, double %t341, double %t342, double %t343, double %t344)
  %t346 = getelementptr inbounds [22 x i8], ptr @.str.18, i64 0, i64 0
  %t347 = load i8, ptr %letter
  %t348 = sext i8 %t347 to i32
  %t349 = load i8, ptr %newline
  %t350 = sext i8 %t349 to i32
  %t351 = load i8, ptr %tab
  %t352 = sext i8 %t351 to i32
  %t353 = load i8, ptr %quote
  %t354 = sext i8 %t353 to i32
  %t355 = load i8, ptr %slash
  %t356 = sext i8 %t355 to i32
  %t357 = load i8, ptr %hexChar
  %t358 = sext i8 %t357 to i32
  %t359 = load i8, ptr %nul
  %t360 = sext i8 %t359 to i32
  %t361 = call i32 (ptr, ...) @printf(ptr %t346, i32 %t348, i32 %t350, i32 %t352, i32 %t354, i32 %t356, i32 %t358, i32 %t360)
  %t362 = getelementptr inbounds [21 x i8], ptr @.str.19, i64 0, i64 0
  %t363 = load ptr, ptr %greeting
  %t364 = load ptr, ptr %escaped
  %t365 = load ptr, ptr %tricky
  %t366 = load ptr, ptr %empty
  %t367 = call i32 (ptr, ...) @printf(ptr %t362, ptr %t363, ptr %t364, ptr %t365, ptr %t366)
  %t368 = getelementptr inbounds [24 x i8], ptr @.str.20, i64 0, i64 0
  %t369 = load i1, ptr %yes
  %t370 = zext i1 %t369 to i32
  %t371 = load i1, ptr %no
  %t372 = zext i1 %t371 to i32
  %t373 = getelementptr inbounds [2 x [2 x i8]], ptr %board, i64 0, i64 0
  %t374 = getelementptr inbounds [2 x i8], ptr %t373, i32 0
  %t375 = getelementptr inbounds [2 x i8], ptr %t374, i64 0, i64 0
  %t376 = getelementptr inbounds i8, ptr %t375, i32 0
  %t377 = load i8, ptr %t376
  %t378 = sext i8 %t377 to i32
  %t379 = getelementptr inbounds [2 x [2 x i8]], ptr %board, i64 0, i64 0
  %t380 = getelementptr inbounds [2 x i8], ptr %t379, i32 1
  %t381 = getelementptr inbounds [2 x i8], ptr %t380, i64 0, i64 0
  %t382 = getelementptr inbounds i8, ptr %t381, i32 1
  %t383 = load i8, ptr %t382
  %t384 = sext i8 %t383 to i32
  %t385 = call i32 (ptr, ...) @printf(ptr %t368, i32 %t370, i32 %t372, i32 %t378, i32 %t384)
  ret i32 0
}

define void @class_3Foo_5setId_i(ptr %this, i32 %v) {
entry:
  %this.addr = alloca ptr
  %v.addr = alloca i32
  store ptr %this, ptr %this.addr
  %t0 = load ptr, ptr %this.addr
  store i32 %v, ptr %v.addr
  %t1 = load i32, ptr %v.addr
  %t2 = getelementptr inbounds %class.Foo, ptr %t0, i32 0, i32 0
  store i32 %t1, ptr %t2
  ret void
}

define i32 @class_3Foo_5getId_v(ptr %this) {
entry:
  %this.addr = alloca ptr
  store ptr %this, ptr %this.addr
  %t0 = load ptr, ptr %this.addr
  %t1 = getelementptr inbounds %class.Foo, ptr %t0, i32 0, i32 0
  %t2 = load i32, ptr %t1
  ret i32 %t2
}

define void @class_3Foo_9setFilled_b(ptr %this, i1 %f) {
entry:
  %this.addr = alloca ptr
  %f.addr = alloca i1
  store ptr %this, ptr %this.addr
  %t0 = load ptr, ptr %this.addr
  store i1 %f, ptr %f.addr
  %t1 = load i1, ptr %f.addr
  %t2 = getelementptr inbounds %class.Foo, ptr %t0, i32 0, i32 2
  store i1 %t1, ptr %t2
  ret void
}

define i1 @class_3Foo_8isFilled_v(ptr %this) {
entry:
  %this.addr = alloca ptr
  store ptr %this, ptr %this.addr
  %t0 = load ptr, ptr %this.addr
  %t1 = getelementptr inbounds %class.Foo, ptr %t0, i32 0, i32 2
  %t2 = load i1, ptr %t1
  ret i1 %t2
}

define i32 @class_3Foo_8checksum_v(ptr %this) {
entry:
  %this.addr = alloca ptr
  store ptr %this, ptr %this.addr
  %t0 = load ptr, ptr %this.addr
  %t1 = getelementptr inbounds %class.Foo, ptr %t0, i32 0, i32 3
  %t2 = load i32, ptr %t1
  %t3 = getelementptr inbounds %class.Foo, ptr %t0, i32 0, i32 1
  %t4 = load i32, ptr %t3
  %t5 = add i32 %t2, %t4
  ret i32 %t5
}

define void @class_3Bar_4init_ii(ptr %this, i32 %a, i32 %b) {
entry:
  %this.addr = alloca ptr
  %a.addr = alloca i32
  %b.addr = alloca i32
  store ptr %this, ptr %this.addr
  %t0 = load ptr, ptr %this.addr
  store i32 %a, ptr %a.addr
  store i32 %b, ptr %b.addr
  %t1 = load i32, ptr %a.addr
  %t2 = getelementptr inbounds %class.Bar, ptr %t0, i32 0, i32 1
  store i32 %t1, ptr %t2
  %t3 = load i32, ptr %b.addr
  %t4 = getelementptr inbounds %class.Bar, ptr %t0, i32 0, i32 2
  store i32 %t3, ptr %t4
  ret void
}

define i32 @class_3Bar_4area_v(ptr %this) {
entry:
  %this.addr = alloca ptr
  store ptr %this, ptr %this.addr
  %t0 = load ptr, ptr %this.addr
  %t1 = getelementptr inbounds %class.Bar, ptr %t0, i32 0, i32 1
  %t2 = load i32, ptr %t1
  %t3 = getelementptr inbounds %class.Bar, ptr %t0, i32 0, i32 2
  %t4 = load i32, ptr %t3
  %t5 = mul i32 %t2, %t4
  ret i32 %t5
}

define i32 @class_3Bar_9perimeter_v(ptr %this) {
entry:
  %this.addr = alloca ptr
  store ptr %this, ptr %this.addr
  %t0 = load ptr, ptr %this.addr
  %t1 = getelementptr inbounds %class.Bar, ptr %t0, i32 0, i32 1
  %t2 = load i32, ptr %t1
  %t3 = getelementptr inbounds %class.Bar, ptr %t0, i32 0, i32 2
  %t4 = load i32, ptr %t3
  %t5 = add i32 %t2, %t4
  %t6 = mul i32 2, %t5
  ret i32 %t6
}

declare ptr @malloc(i64)

declare void @free(ptr)

define internal i32 @__lambda_262_17(ptr %__closure, i32 %v) {
entry:
  %v.addr = alloca i32
  store i32 %v, ptr %v.addr
  %t0 = load i32, ptr %v.addr
  %t1 = load i32, ptr %v.addr
  %t2 = mul i32 %t0, %t1
  %t3 = load i32, ptr %v.addr
  %t4 = mul i32 %t2, %t3
  ret i32 %t4
}

define internal i32 @__lambda_265_22(ptr %__closure, i32 %v) {
entry:
  %v.addr = alloca i32
  %t0 = getelementptr inbounds %closure.265.22, ptr %__closure, i32 0, i32 0
  store i32 %v, ptr %v.addr
  %t1 = load i32, ptr %v.addr
  %t2 = load i32, ptr %t0
  %t3 = add i32 %t1, %t2
  ret i32 %t3
}

define internal i32 @__lambda_266_22(ptr %__closure, i32 %v) {
entry:
  %v.addr = alloca i32
  %t0 = getelementptr inbounds %closure.266.22, ptr %__closure, i32 0, i32 0
  store i32 %v, ptr %v.addr
  %t1 = load i32, ptr %v.addr
  %t2 = load ptr, ptr %t0
  %t3 = load i32, ptr %t2
  %t4 = add i32 %t1, %t3
  ret i32 %t4
}

define internal i32 @__lambda_267_22(ptr %__closure, i32 %v) {
entry:
  %v.addr = alloca i32
  %t0 = getelementptr inbounds %closure.267.22, ptr %__closure, i32 0, i32 0
  store i32 %v, ptr %v.addr
  %t1 = load i32, ptr %v.addr
  %t2 = load i32, ptr %t0
  %t3 = add i32 %t1, %t2
  ret i32 %t3
}

define internal i32 @__lambda_268_22(ptr %__closure, i32 %v) {
entry:
  %v.addr = alloca i32
  %t0 = getelementptr inbounds %closure.268.22, ptr %__closure, i32 0, i32 0
  store i32 %v, ptr %v.addr
  %t1 = load i32, ptr %v.addr
  %t2 = load ptr, ptr %t0
  %t3 = load i32, ptr %t2
  %t4 = add i32 %t1, %t3
  ret i32 %t4
}

define internal i32 @__lambda_269_22(ptr %__closure, i32 %v) {
entry:
  %v.addr = alloca i32
  %t0 = getelementptr inbounds %closure.269.22, ptr %__closure, i32 0, i32 0
  %t1 = getelementptr inbounds %closure.269.22, ptr %__closure, i32 0, i32 1
  store i32 %v, ptr %v.addr
  %t2 = load i32, ptr %v.addr
  %t3 = load i32, ptr %t0
  %t4 = add i32 %t2, %t3
  %t5 = load ptr, ptr %t1
  %t6 = load i32, ptr %t5
  %t7 = add i32 %t4, %t6
  ret i32 %t7
}
