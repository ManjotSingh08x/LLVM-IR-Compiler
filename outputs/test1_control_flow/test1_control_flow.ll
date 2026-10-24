; ModuleID = 'test/test1_control_flow.ckp'
source_filename = "test/test1_control_flow.ckp"

define i32 @global_8classify_i(i32 %n) {
entry:
  %n.addr = alloca i32
  store i32 %n, ptr %n.addr
  %t0 = load i32, ptr %n.addr
  %t1 = icmp slt i32 %t0, 0
  br i1 %t1, label %if.then, label %if.else

if.then:  ; preds = %entry
  %t2 = sub i32 0, 1
  ret i32 %t2

if.else:  ; preds = %entry
  %t3 = load i32, ptr %n.addr
  %t4 = icmp eq i32 %t3, 0
  br i1 %t4, label %if.then.1, label %if.else.1

if.then.1:  ; preds = %if.else
  ret i32 0

if.else.1:  ; preds = %if.else
  ret i32 1
}

define i32 @main() {
entry:
  %a = alloca i32
  %b = alloca i32
  %c = alloca i32
  %t = alloca i1
  %and.result = alloca i1
  %or.result = alloca i1
  %cond.result = alloca ptr
  %i = alloca i32
  %sum = alloca i32
  store i32 10, ptr %a
  store i32 3, ptr %b
  %t0 = load i32, ptr %a
  %t1 = load i32, ptr %b
  %t2 = add i32 %t0, %t1
  store i32 %t2, ptr %c
  %t3 = load i32, ptr %a
  %t4 = load i32, ptr %b
  %t5 = sub i32 %t3, %t4
  store i32 %t5, ptr %c
  %t6 = load i32, ptr %a
  %t7 = load i32, ptr %b
  %t8 = mul i32 %t6, %t7
  store i32 %t8, ptr %c
  %t9 = load i32, ptr %a
  %t10 = load i32, ptr %b
  %t11 = sdiv i32 %t9, %t10
  store i32 %t11, ptr %c
  %t12 = load i32, ptr %a
  %t13 = load i32, ptr %b
  %t14 = srem i32 %t12, %t13
  store i32 %t14, ptr %c
  %t15 = load i32, ptr %a
  %t16 = load i32, ptr %c
  %t17 = add i32 %t16, %t15
  store i32 %t17, ptr %c
  %t18 = load i32, ptr %a
  %t19 = load i32, ptr %c
  %t20 = sub i32 %t19, %t18
  store i32 %t20, ptr %c
  %t21 = load i32, ptr %a
  %t22 = load i32, ptr %c
  %t23 = mul i32 %t22, %t21
  store i32 %t23, ptr %c
  %t24 = load i32, ptr %a
  %t25 = load i32, ptr %c
  %t26 = sdiv i32 %t25, %t24
  store i32 %t26, ptr %c
  %t27 = load i32, ptr %a
  %t28 = load i32, ptr %c
  %t29 = srem i32 %t28, %t27
  store i32 %t29, ptr %c
  %t30 = load i32, ptr %a
  %t31 = add i32 %t30, 1
  store i32 %t31, ptr %a
  %t32 = load i32, ptr %a
  %t33 = sub i32 %t32, 1
  store i32 %t33, ptr %a
  %t34 = load i32, ptr %a
  %t35 = add i32 %t34, 1
  store i32 %t35, ptr %a
  %t36 = load i32, ptr %a
  %t37 = sub i32 %t36, 1
  store i32 %t37, ptr %a
  %t38 = load i32, ptr %a
  %t39 = load i32, ptr %b
  %t40 = icmp eq i32 %t38, %t39
  store i1 %t40, ptr %t
  %t41 = load i32, ptr %a
  %t42 = load i32, ptr %b
  %t43 = icmp ne i32 %t41, %t42
  store i1 %t43, ptr %t
  %t44 = load i32, ptr %a
  %t45 = load i32, ptr %b
  %t46 = icmp slt i32 %t44, %t45
  store i1 %t46, ptr %t
  %t47 = load i32, ptr %a
  %t48 = load i32, ptr %b
  %t49 = icmp sgt i32 %t47, %t48
  store i1 %t49, ptr %t
  %t50 = load i32, ptr %a
  %t51 = load i32, ptr %b
  %t52 = icmp sle i32 %t50, %t51
  store i1 %t52, ptr %t
  %t53 = load i32, ptr %a
  %t54 = load i32, ptr %b
  %t55 = icmp sge i32 %t53, %t54
  store i1 %t55, ptr %t
  %t56 = load i32, ptr %a
  %t57 = icmp ne i32 %t56, 0
  br i1 %t57, label %land.rhs, label %land.short

land.short:  ; preds = %entry
  store i1 false, ptr %and.result
  br label %land.end

land.rhs:  ; preds = %entry
  %t58 = load i32, ptr %b
  %t59 = icmp ne i32 %t58, 0
  store i1 %t59, ptr %and.result
  br label %land.end

land.end:  ; preds = %land.short, %land.rhs
  %t60 = load i1, ptr %and.result
  store i1 %t60, ptr %t
  %t61 = load i32, ptr %a
  %t62 = icmp ne i32 %t61, 0
  br i1 %t62, label %lor.short, label %lor.rhs

lor.short:  ; preds = %land.end
  store i1 true, ptr %or.result
  br label %lor.end

lor.rhs:  ; preds = %land.end
  %t63 = load i32, ptr %b
  %t64 = icmp ne i32 %t63, 0
  store i1 %t64, ptr %or.result
  br label %lor.end

lor.end:  ; preds = %lor.short, %lor.rhs
  %t65 = load i1, ptr %or.result
  store i1 %t65, ptr %t
  %t66 = load i32, ptr %a
  %t67 = icmp ne i32 %t66, 0
  %t68 = xor i1 %t67, true
  store i1 %t68, ptr %t
  %t69 = load i32, ptr %a
  %t70 = load i32, ptr %b
  %t71 = and i32 %t69, %t70
  store i32 %t71, ptr %c
  %t72 = load i32, ptr %a
  %t73 = load i32, ptr %b
  %t74 = or i32 %t72, %t73
  store i32 %t74, ptr %c
  %t75 = load i32, ptr %a
  %t76 = load i32, ptr %b
  %t77 = xor i32 %t75, %t76
  store i32 %t77, ptr %c
  %t78 = load i32, ptr %a
  %t79 = xor i32 %t78, -1
  store i32 %t79, ptr %c
  %t80 = load i32, ptr %a
  %t81 = shl i32 %t80, 2
  store i32 %t81, ptr %c
  %t82 = load i32, ptr %a
  %t83 = ashr i32 %t82, 2
  store i32 %t83, ptr %c
  %t84 = load i32, ptr %a
  %t85 = load i32, ptr %b
  %t86 = icmp sgt i32 %t84, %t85
  br i1 %t86, label %cond.true, label %cond.false

cond.true:  ; preds = %lor.end
  store ptr %a, ptr %cond.result
  br label %cond.end

cond.false:  ; preds = %lor.end
  store ptr %b, ptr %cond.result
  br label %cond.end

cond.end:  ; preds = %cond.true, %cond.false
  %t87 = load ptr, ptr %cond.result
  %t88 = load i32, ptr %t87
  store i32 %t88, ptr %c
  store i32 0, ptr %sum
  store i32 0, ptr %i
  br label %for.cond

for.cond:  ; preds = %cond.end, %for.inc
  %t89 = load i32, ptr %i
  %t90 = icmp slt i32 %t89, 10
  br i1 %t90, label %for.body, label %for.end

for.body:  ; preds = %for.cond
  %t91 = load i32, ptr %i
  %t92 = srem i32 %t91, 2
  %t93 = icmp eq i32 %t92, 0
  br i1 %t93, label %if.then, label %if.end

if.then:  ; preds = %for.body
  br label %for.inc

if.end:  ; preds = %for.body
  %t94 = load i32, ptr %i
  %t95 = icmp sgt i32 %t94, 7
  br i1 %t95, label %if.then.1, label %if.end.1

if.then.1:  ; preds = %if.end
  br label %for.end

if.end.1:  ; preds = %if.end
  %t96 = load i32, ptr %i
  %t97 = load i32, ptr %sum
  %t98 = add i32 %t97, %t96
  store i32 %t98, ptr %sum
  br label %for.inc

for.inc:  ; preds = %if.then, %if.end.1
  %t99 = load i32, ptr %i
  %t100 = add i32 %t99, 1
  store i32 %t100, ptr %i
  br label %for.cond

for.end:  ; preds = %for.cond, %if.then.1
  br label %while.cond

while.cond:  ; preds = %for.end, %while.body
  %t101 = load i32, ptr %sum
  %t102 = icmp sgt i32 %t101, 0
  br i1 %t102, label %while.body, label %while.end

while.body:  ; preds = %while.cond
  %t103 = load i32, ptr %sum
  %t104 = sub i32 %t103, 1
  store i32 %t104, ptr %sum
  br label %while.cond

while.end:  ; preds = %while.cond
  br label %do.body

do.body:  ; preds = %while.end, %do.cond
  %t105 = load i32, ptr %sum
  %t106 = add i32 %t105, 2
  store i32 %t106, ptr %sum
  br label %do.cond

do.cond:  ; preds = %do.body
  %t107 = load i32, ptr %sum
  %t108 = icmp slt i32 %t107, 10
  br i1 %t108, label %do.body, label %do.end

do.end:  ; preds = %do.cond
  store i32 0, ptr %i
  br label %top

top:  ; preds = %do.end, %if.then.2
  %t109 = load i32, ptr %i
  %t110 = add i32 %t109, 1
  store i32 %t110, ptr %i
  %t111 = load i32, ptr %i
  %t112 = icmp slt i32 %t111, 3
  br i1 %t112, label %if.then.2, label %if.end.2

if.then.2:  ; preds = %top
  br label %top

if.end.2:  ; preds = %top
  %t113 = load i32, ptr %sum
  %t114 = call i32 @global_8classify_i(i32 %t113)
  ret i32 %t114
}
