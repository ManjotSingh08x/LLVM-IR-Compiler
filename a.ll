; ModuleID = '/home/manjot/files/code/compilerdesign/LLVM-IR-Compiler/test/regression/stress_structures.ckp'
source_filename = "/home/manjot/files/code/compilerdesign/LLVM-IR-Compiler/test/regression/stress_structures.ckp"

%struct.Node = type { i32, ptr }
%struct.Tree = type { i32, i32, ptr, ptr }
%class.Stack = type { [64 x i32], i32 }
%class.Ring = type { [8 x i32], i32, i32 }

@.str = private unnamed_addr constant [21 x i8] c"list %d %d first %d\0A\00"
@.str.1 = private unnamed_addr constant [10 x i8] c"stack %d\0A\00"
@.str.2 = private unnamed_addr constant [12 x i8] c"ring %d %d\0A\00"
@.str.3 = private unnamed_addr constant [23 x i8] c"tree depth %d walk %d\0A\00"

declare i32 @printf(ptr, ...)

define ptr @global_9pushFront_P4Nodei(ptr %list, i32 %v) {
entry:
  %t0 = getelementptr %struct.Node, ptr null, i64 1
  %t1 = ptrtoint ptr %t0 to i64
  %t2 = call ptr @malloc(i64 %t1)
  %t5 = getelementptr inbounds %struct.Node, ptr %t2, i32 0, i32 0
  store i32 %v, ptr %t5
  %t8 = getelementptr inbounds %struct.Node, ptr %t2, i32 0, i32 1
  store ptr %list, ptr %t8
  ret ptr %t2
}

define ptr @global_7reverse_P4Node(ptr %list) {
entry:
  %t1.1 = icmp ne ptr %list, null
  br i1 %t1.1, label %while.body, label %while.end

while.body:  ; preds = %entry, %while.body
  %prev.1 = phi ptr [ %list.1, %while.body ], [ null, %entry ]
  %list.1 = phi ptr [ %t4, %while.body ], [ %list, %entry ]
  %t3 = getelementptr inbounds %struct.Node, ptr %list.1, i32 0, i32 1
  %t4 = load ptr, ptr %t3
  store ptr %prev.1, ptr %t3
  %t1 = icmp ne ptr %t4, null
  br i1 %t1, label %while.body, label %while.end.loopexit.1

while.end.loopexit.1:  ; preds = %while.body
  br label %while.end

while.end:  ; preds = %entry, %while.end.loopexit.1
  %prev.2 = phi ptr [ null, %entry ], [ %list.1, %while.end.loopexit.1 ]
  ret ptr %prev.2
}

define i32 @global_7sumList_P4Node(ptr %list) {
entry:
  %t1.1 = icmp ne ptr %list, null
  br i1 %t1.1, label %while.body, label %while.end

while.body:  ; preds = %entry, %while.body
  %pos.1 = phi i32 [ %t10, %while.body ], [ 1, %entry ]
  %s.1 = phi i32 [ %t8, %while.body ], [ 0, %entry ]
  %list.1 = phi ptr [ %t13, %while.body ], [ %list, %entry ]
  %t3 = getelementptr inbounds %struct.Node, ptr %list.1, i32 0, i32 0
  %t4 = load i32, ptr %t3
  %t6 = mul i32 %t4, %pos.1
  %t8 = add i32 %s.1, %t6
  %t10 = add i32 %pos.1, 1
  %t12 = getelementptr inbounds %struct.Node, ptr %list.1, i32 0, i32 1
  %t13 = load ptr, ptr %t12
  %t1 = icmp ne ptr %t13, null
  br i1 %t1, label %while.body, label %while.end.loopexit.1

while.end.loopexit.1:  ; preds = %while.body
  br label %while.end

while.end:  ; preds = %entry, %while.end.loopexit.1
  %s.2 = phi i32 [ 0, %entry ], [ %t8, %while.end.loopexit.1 ]
  ret i32 %s.2
}

define void @global_8freeList_P4Node(ptr %list) {
entry:
  %t1.1 = icmp ne ptr %list, null
  br i1 %t1.1, label %while.body, label %while.end

while.body:  ; preds = %entry, %while.body
  %list.1 = phi ptr [ %t4, %while.body ], [ %list, %entry ]
  %t3 = getelementptr inbounds %struct.Node, ptr %list.1, i32 0, i32 1
  %t4 = load ptr, ptr %t3
  call void @free(ptr %list.1)
  %t1 = icmp ne ptr %t4, null
  br i1 %t1, label %while.body, label %while.end

while.end:  ; preds = %entry, %while.body
  ret void
}

define ptr @global_6insert_P4Treei(ptr %t, i32 %key) {
entry:
  %t1 = icmp eq ptr %t, null
  br i1 %t1, label %if.then, label %if.end

if.then:  ; preds = %entry
  %t2 = getelementptr %struct.Tree, ptr null, i64 1
  %t3 = ptrtoint ptr %t2 to i64
  %t4 = call ptr @malloc(i64 %t3)
  %t7 = getelementptr inbounds %struct.Tree, ptr %t4, i32 0, i32 0
  store i32 %key, ptr %t7
  %t9 = getelementptr inbounds %struct.Tree, ptr %t4, i32 0, i32 1
  store i32 1, ptr %t9
  %t11 = getelementptr inbounds %struct.Tree, ptr %t4, i32 0, i32 2
  store ptr null, ptr %t11
  %t13 = getelementptr inbounds %struct.Tree, ptr %t4, i32 0, i32 3
  store ptr null, ptr %t13
  ret ptr %t4

if.end:  ; preds = %entry
  %t17 = getelementptr inbounds %struct.Tree, ptr %t, i32 0, i32 0
  %t18 = load i32, ptr %t17
  %t19 = icmp eq i32 %key, %t18
  br i1 %t19, label %if.then.1, label %if.else

if.then.1:  ; preds = %if.end
  %t21 = getelementptr inbounds %struct.Tree, ptr %t, i32 0, i32 1
  %t22 = load i32, ptr %t21
  %t23 = add i32 %t22, 1
  store i32 %t23, ptr %t21
  br label %if.end.1

if.else:  ; preds = %if.end
  %t27 = load i32, ptr %t17
  %t28 = icmp slt i32 %key, %t27
  br i1 %t28, label %if.then.2, label %if.else.1

if.then.2:  ; preds = %if.else
  %t30 = getelementptr inbounds %struct.Tree, ptr %t, i32 0, i32 2
  %t31 = load ptr, ptr %t30
  %t33 = call ptr @global_6insert_P4Treei(ptr %t31, i32 %key)
  store ptr %t33, ptr %t30
  br label %if.end.1

if.else.1:  ; preds = %if.else
  %t37 = getelementptr inbounds %struct.Tree, ptr %t, i32 0, i32 3
  %t38 = load ptr, ptr %t37
  %t40 = call ptr @global_6insert_P4Treei(ptr %t38, i32 %key)
  store ptr %t40, ptr %t37
  br label %if.end.1

if.end.1:  ; preds = %if.then.1, %if.then.2, %if.else.1
  ret ptr %t
}

define i32 @global_5depth_P4Tree(ptr %t) {
entry:
  %l = alloca i32
  %r = alloca i32
  %t1 = icmp eq ptr %t, null
  br i1 %t1, label %if.then, label %if.end

if.then:  ; preds = %entry
  ret i32 0

if.end:  ; preds = %entry
  %t3 = getelementptr inbounds %struct.Tree, ptr %t, i32 0, i32 2
  %t4 = load ptr, ptr %t3
  %t5 = call i32 @global_5depth_P4Tree(ptr %t4)
  store i32 %t5, ptr %l
  %t7 = getelementptr inbounds %struct.Tree, ptr %t, i32 0, i32 3
  %t8 = load ptr, ptr %t7
  %t9 = call i32 @global_5depth_P4Tree(ptr %t8)
  store i32 %t9, ptr %r
  %t10 = load i32, ptr %l
  %t12 = icmp sgt i32 %t10, %t9
  br i1 %t12, label %cond.end, label %cond.false

cond.false:  ; preds = %if.end
  br label %cond.end

cond.end:  ; preds = %if.end, %cond.false
  %cond.result.1 = phi ptr [ %r, %cond.false ], [ %l, %if.end ]
  %t14 = load i32, ptr %cond.result.1
  %t15 = add i32 1, %t14
  ret i32 %t15
}

define i32 @global_4walk_P4Treei(ptr %t, i32 %acc) {
entry:
  %t1 = icmp eq ptr %t, null
  br i1 %t1, label %if.then, label %if.end

if.then:  ; preds = %entry
  ret i32 %acc

if.end:  ; preds = %entry
  %t4 = getelementptr inbounds %struct.Tree, ptr %t, i32 0, i32 2
  %t5 = load ptr, ptr %t4
  %t7 = call i32 @global_4walk_P4Treei(ptr %t5, i32 %acc)
  %t9 = mul i32 %t7, 7
  %t11 = getelementptr inbounds %struct.Tree, ptr %t, i32 0, i32 0
  %t12 = load i32, ptr %t11
  %t14 = getelementptr inbounds %struct.Tree, ptr %t, i32 0, i32 1
  %t15 = load i32, ptr %t14
  %t16 = mul i32 %t12, %t15
  %t17 = add i32 %t9, %t16
  %t18 = srem i32 %t17, 1000003
  %t20 = getelementptr inbounds %struct.Tree, ptr %t, i32 0, i32 3
  %t21 = load ptr, ptr %t20
  %t23 = call i32 @global_4walk_P4Treei(ptr %t21, i32 %t18)
  ret i32 %t23
}

define void @global_8freeTree_P4Tree(ptr %t) {
entry:
  %t1 = icmp eq ptr %t, null
  br i1 %t1, label %if.then, label %if.end

if.then:  ; preds = %entry
  ret void

if.end:  ; preds = %entry
  %t3 = getelementptr inbounds %struct.Tree, ptr %t, i32 0, i32 2
  %t4 = load ptr, ptr %t3
  call void @global_8freeTree_P4Tree(ptr %t4)
  %t6 = getelementptr inbounds %struct.Tree, ptr %t, i32 0, i32 3
  %t7 = load ptr, ptr %t6
  call void @global_8freeTree_P4Tree(ptr %t7)
  call void @free(ptr %t)
  ret void
}

define i32 @main() {
entry:
  %s = alloca %class.Stack
  %r = alloca %class.Ring
  %keys = alloca [12 x i32]
  %t0.2 = getelementptr %struct.Node, ptr null, i64 1
  %t1.2 = ptrtoint ptr %t0.2 to i64
  br label %for.body

for.body:  ; preds = %entry, %for.body
  %i.5 = phi i32 [ 1, %entry ], [ %t9, %for.body ]
  %list.1 = phi ptr [ null, %entry ], [ %t2.1, %for.body ]
  %t5 = mul i32 %i.5, %i.5
  %t6 = srem i32 %t5, 37
  %t2.1 = call ptr @malloc(i64 %t1.2)
  %t5.1 = getelementptr inbounds %struct.Node, ptr %t2.1, i32 0, i32 0
  store i32 %t6, ptr %t5.1
  %t8.1 = getelementptr inbounds %struct.Node, ptr %t2.1, i32 0, i32 1
  store ptr %list.1, ptr %t8.1
  %t9 = add i32 %i.5, 1
  %t1 = icmp sle i32 %t9, 50
  br i1 %t1, label %for.body, label %for.end

for.end:  ; preds = %for.body
  %t1.1.1 = icmp ne ptr %t2.1, null
  br i1 %t1.1.1, label %while.body.2, label %while.end.2

while.body.2:  ; preds = %for.end, %while.body.2
  %pos.2 = phi i32 [ %t10.1, %while.body.2 ], [ 1, %for.end ]
  %s.1.1 = phi i32 [ %t8.2, %while.body.2 ], [ 0, %for.end ]
  %list.addr.1.1 = phi ptr [ %t13.1, %while.body.2 ], [ %t2.1, %for.end ]
  %t3.2 = getelementptr inbounds %struct.Node, ptr %list.addr.1.1, i32 0, i32 0
  %t4.2 = load i32, ptr %t3.2
  %t6.2 = mul i32 %t4.2, %pos.2
  %t8.2 = add i32 %s.1.1, %t6.2
  %t10.1 = add i32 %pos.2, 1
  %t12.1 = getelementptr inbounds %struct.Node, ptr %list.addr.1.1, i32 0, i32 1
  %t13.1 = load ptr, ptr %t12.1
  %t1.3 = icmp ne ptr %t13.1, null
  br i1 %t1.3, label %while.body.2, label %while.end.2.loopexit

while.end.2.loopexit:  ; preds = %while.body.2
  br label %while.end.2

while.end.2:  ; preds = %for.end, %while.end.2.loopexit
  %s.1.2 = phi i32 [ 0, %for.end ], [ %t8.2, %while.end.2.loopexit ]
  br i1 %t1.1.1, label %while.body.3, label %while.end.3

while.body.3:  ; preds = %while.end.2, %while.body.3
  %prev.1 = phi ptr [ %list.addr.2.1, %while.body.3 ], [ null, %while.end.2 ]
  %list.addr.2.1 = phi ptr [ %t4.3, %while.body.3 ], [ %t2.1, %while.end.2 ]
  %t3.3 = getelementptr inbounds %struct.Node, ptr %list.addr.2.1, i32 0, i32 1
  %t4.3 = load ptr, ptr %t3.3
  store ptr %prev.1, ptr %t3.3
  %t1.4 = icmp ne ptr %t4.3, null
  br i1 %t1.4, label %while.body.3, label %while.end.3.loopexit

while.end.3.loopexit:  ; preds = %while.body.3
  br label %while.end.3

while.end.3:  ; preds = %while.end.2, %while.end.3.loopexit
  %prev.2 = phi ptr [ null, %while.end.2 ], [ %list.addr.2.1, %while.end.3.loopexit ]
  %t1.1.3 = icmp ne ptr %prev.2, null
  br i1 %t1.1.3, label %while.body.4, label %while.end.4

while.body.4:  ; preds = %while.end.3, %while.body.4
  %pos.1.1 = phi i32 [ %t10.3, %while.body.4 ], [ 1, %while.end.3 ]
  %s.2.1 = phi i32 [ %t8.4, %while.body.4 ], [ 0, %while.end.3 ]
  %list.addr.3.1 = phi ptr [ %t13.2, %while.body.4 ], [ %prev.2, %while.end.3 ]
  %t3.4 = getelementptr inbounds %struct.Node, ptr %list.addr.3.1, i32 0, i32 0
  %t4.4 = load i32, ptr %t3.4
  %t6.4 = mul i32 %t4.4, %pos.1.1
  %t8.4 = add i32 %s.2.1, %t6.4
  %t10.3 = add i32 %pos.1.1, 1
  %t12.2 = getelementptr inbounds %struct.Node, ptr %list.addr.3.1, i32 0, i32 1
  %t13.2 = load ptr, ptr %t12.2
  %t1.5 = icmp ne ptr %t13.2, null
  br i1 %t1.5, label %while.body.4, label %while.end.4.loopexit

while.end.4.loopexit:  ; preds = %while.body.4
  br label %while.end.4

while.end.4:  ; preds = %while.end.3, %while.end.4.loopexit
  %s.2.2 = phi i32 [ 0, %while.end.3 ], [ %t8.4, %while.end.4.loopexit ]
  %t14 = getelementptr inbounds [21 x i8], ptr @.str, i64 0, i64 0
  %t19 = getelementptr inbounds %struct.Node, ptr %prev.2, i32 0, i32 0
  %t20 = load i32, ptr %t19
  %t21 = call i32 (ptr, ...) @printf(ptr %t14, i32 %s.1.2, i32 %s.2.2, i32 %t20)
  br i1 %t1.1.3, label %while.body.5, label %entry.6

while.body.5:  ; preds = %while.end.4, %while.body.5
  %list.addr.4.1 = phi ptr [ %t4.5, %while.body.5 ], [ %prev.2, %while.end.4 ]
  %t3.5 = getelementptr inbounds %struct.Node, ptr %list.addr.4.1, i32 0, i32 1
  %t4.5 = load ptr, ptr %t3.5
  call void @free(ptr %list.addr.4.1)
  %t1.6 = icmp ne ptr %t4.5, null
  br i1 %t1.6, label %while.body.5, label %entry.6

entry.6:  ; preds = %while.end.4, %while.body.5
  %t1.7 = getelementptr inbounds %class.Stack, ptr %s, i32 0, i32 1
  store i32 0, ptr %t1.7
  %t2.6 = getelementptr inbounds %class.Stack, ptr %s, i32 0, i32 0
  %t3.6 = getelementptr inbounds [64 x i32], ptr %t2.6, i64 0, i64 0
  br label %for.body.1

for.body.1:  ; preds = %entry.6, %for.body.1
  %t26.iv = phi i32 [ 0, %entry.6 ], [ %t26.next, %for.body.1 ]
  %i.1.1 = phi i32 [ 0, %entry.6 ], [ %t28, %for.body.1 ]
  %t5.6 = load i32, ptr %t1.7
  %t6.6 = getelementptr inbounds i32, ptr %t3.6, i32 %t5.6
  store i32 %t26.iv, ptr %t6.6
  %t8.5 = load i32, ptr %t1.7
  %t9.5 = add i32 %t8.5, 1
  store i32 %t9.5, ptr %t1.7
  %t28 = add i32 %i.1.1, 1
  %t24 = icmp slt i32 %t28, 20
  %t26.next = add i32 %t26.iv, 3
  br i1 %t24, label %for.body.1, label %for.end.1

for.end.1:  ; preds = %for.body.1
  br label %entry.8

entry.8:  ; preds = %for.end.1, %while.body
  %popped.1 = phi i32 [ 0, %for.end.1 ], [ %t35, %while.body ]
  %t2.7 = load i32, ptr %t1.7
  %t3.7 = icmp eq i32 %t2.7, 0
  %t30 = xor i1 %t3.7, true
  br i1 %t30, label %while.body, label %while.end

while.body:  ; preds = %entry.8
  %t32 = shl i32 %popped.1, 1
  %t33 = srem i32 %t32, 10007
  %t2.8 = load i32, ptr %t1.7
  %t3.8 = sub i32 %t2.8, 1
  store i32 %t3.8, ptr %t1.7
  %t8.6 = getelementptr inbounds i32, ptr %t3.6, i32 %t3.8
  %t9.6 = load i32, ptr %t8.6
  %t35 = add i32 %t33, %t9.6
  br label %entry.8

while.end:  ; preds = %entry.8
  %t36 = getelementptr inbounds [10 x i8], ptr @.str.1, i64 0, i64 0
  %t38 = call i32 (ptr, ...) @printf(ptr %t36, i32 %popped.1)
  %t1.11 = getelementptr inbounds %class.Ring, ptr %r, i32 0, i32 1
  store i32 0, ptr %t1.11
  %t2.9 = getelementptr inbounds %class.Ring, ptr %r, i32 0, i32 2
  store i32 0, ptr %t2.9
  %t5.8 = getelementptr inbounds %class.Ring, ptr %r, i32 0, i32 0
  %t6.8 = getelementptr inbounds [8 x i32], ptr %t5.8, i64 0, i64 0
  br label %for.body.2

for.body.2:  ; preds = %while.end, %for.inc.2
  %i.2.1 = phi i32 [ 0, %while.end ], [ %t53, %for.inc.2 ]
  %drained.4 = phi i32 [ 0, %while.end ], [ %drained.1, %for.inc.2 ]
  %rejected.2 = phi i32 [ 0, %while.end ], [ %rejected.1, %for.inc.2 ]
  %t2.10 = load i32, ptr %t2.9
  %t3.9 = icmp eq i32 %t2.10, 8
  br i1 %t3.9, label %class_4Ring_3put_i.exit, label %if.end.2

if.end.2:  ; preds = %for.body.2
  %t8.7 = load i32, ptr %t1.11
  %t10.4 = load i32, ptr %t2.9
  %t11.3 = add i32 %t8.7, %t10.4
  %t12.3 = srem i32 %t11.3, 8
  %t13.3 = getelementptr inbounds i32, ptr %t6.8, i32 %t12.3
  store i32 %i.2.1, ptr %t13.3
  %t15.1 = load i32, ptr %t2.9
  %t16.1 = add i32 %t15.1, 1
  store i32 %t16.1, ptr %t2.9
  br label %class_4Ring_3put_i.exit

class_4Ring_3put_i.exit:  ; preds = %for.body.2, %if.end.2
  %class_4Ring_3put_i.ret.1 = phi i1 [ true, %if.end.2 ], [ false, %for.body.2 ]
  %t43 = xor i1 %class_4Ring_3put_i.ret.1, true
  br i1 %t43, label %if.then, label %if.end

if.then:  ; preds = %class_4Ring_3put_i.exit
  %t45 = add i32 %rejected.2, 1
  br label %if.end

if.end:  ; preds = %class_4Ring_3put_i.exit, %if.then
  %rejected.1 = phi i32 [ %rejected.2, %class_4Ring_3put_i.exit ], [ %t45, %if.then ]
  %t47 = srem i32 %i.2.1, 3
  %t48 = icmp eq i32 %t47, 2
  br i1 %t48, label %entry.12, label %for.inc.2

entry.12:  ; preds = %if.end
  %t4.9 = load i32, ptr %t1.11
  %t5.9 = getelementptr inbounds i32, ptr %t6.8, i32 %t4.9
  %t6.9 = load i32, ptr %t5.9
  %t9.8 = add i32 %t4.9, 1
  %t10.5 = srem i32 %t9.8, 8
  store i32 %t10.5, ptr %t1.11
  %t13.4 = load i32, ptr %t2.9
  %t14.4 = sub i32 %t13.4, 1
  store i32 %t14.4, ptr %t2.9
  %t51 = add i32 %drained.4, %t6.9
  br label %for.inc.2

for.inc.2:  ; preds = %if.end, %entry.12
  %drained.1 = phi i32 [ %drained.4, %if.end ], [ %t51, %entry.12 ]
  %t53 = add i32 %i.2.1, 1
  %t40 = icmp slt i32 %t53, 30
  br i1 %t40, label %for.body.2, label %while.cond.1.loopexit

while.cond.1.loopexit:  ; preds = %for.inc.2
  %t55.1 = load i32, ptr %t2.9
  %t56.1 = icmp sgt i32 %t55.1, 0
  br i1 %t56.1, label %entry.13.preheader, label %while.end.1

entry.13.preheader:  ; preds = %while.cond.1.loopexit
  br label %entry.13

entry.13:  ; preds = %entry.13.preheader, %entry.13
  %drained.2 = phi i32 [ %t60, %entry.13 ], [ %drained.1, %entry.13.preheader ]
  %t4.10 = load i32, ptr %t1.11
  %t5.10 = getelementptr inbounds i32, ptr %t6.8, i32 %t4.10
  %t6.10 = load i32, ptr %t5.10
  %t9.9 = add i32 %t4.10, 1
  %t10.6 = srem i32 %t9.9, 8
  store i32 %t10.6, ptr %t1.11
  %t13.5 = load i32, ptr %t2.9
  %t14.5 = sub i32 %t13.5, 1
  store i32 %t14.5, ptr %t2.9
  %t58 = shl i32 %t6.10, 1
  %t60 = add i32 %drained.2, %t58
  %t55 = load i32, ptr %t2.9
  %t56 = icmp sgt i32 %t55, 0
  br i1 %t56, label %entry.13, label %while.end.1.loopexit.1

while.end.1.loopexit.1:  ; preds = %entry.13
  br label %while.end.1

while.end.1:  ; preds = %while.cond.1.loopexit, %while.end.1.loopexit.1
  %drained.3 = phi i32 [ %drained.1, %while.cond.1.loopexit ], [ %t60, %while.end.1.loopexit.1 ]
  %t61 = getelementptr inbounds [12 x i8], ptr @.str.2, i64 0, i64 0
  %t64 = call i32 (ptr, ...) @printf(ptr %t61, i32 %rejected.1, i32 %drained.3)
  %t65 = getelementptr inbounds [12 x i32], ptr %keys, i64 0, i64 0
  store i32 50, ptr %t65
  %t66 = getelementptr inbounds [12 x i32], ptr %keys, i64 0, i64 1
  store i32 30, ptr %t66
  %t67 = getelementptr inbounds [12 x i32], ptr %keys, i64 0, i64 2
  store i32 70, ptr %t67
  %t68 = getelementptr inbounds [12 x i32], ptr %keys, i64 0, i64 3
  store i32 20, ptr %t68
  %t69 = getelementptr inbounds [12 x i32], ptr %keys, i64 0, i64 4
  store i32 40, ptr %t69
  %t70 = getelementptr inbounds [12 x i32], ptr %keys, i64 0, i64 5
  store i32 60, ptr %t70
  %t71 = getelementptr inbounds [12 x i32], ptr %keys, i64 0, i64 6
  store i32 80, ptr %t71
  %t72 = getelementptr inbounds [12 x i32], ptr %keys, i64 0, i64 7
  store i32 30, ptr %t72
  %t73 = getelementptr inbounds [12 x i32], ptr %keys, i64 0, i64 8
  store i32 65, ptr %t73
  %t74 = getelementptr inbounds [12 x i32], ptr %keys, i64 0, i64 9
  store i32 20, ptr %t74
  %t75 = getelementptr inbounds [12 x i32], ptr %keys, i64 0, i64 10
  store i32 85, ptr %t75
  %t76 = getelementptr inbounds [12 x i32], ptr %keys, i64 0, i64 11
  store i32 50, ptr %t76
  %t82.start = getelementptr inbounds i32, ptr %t65, i32 0
  br label %for.body.3

for.body.3:  ; preds = %while.end.1, %for.body.3
  %t82.iv = phi ptr [ %t82.start, %while.end.1 ], [ %t82.next, %for.body.3 ]
  %i.3.1 = phi i32 [ 0, %while.end.1 ], [ %t86, %for.body.3 ]
  %root.1 = phi ptr [ null, %while.end.1 ], [ %t84, %for.body.3 ]
  %t83 = load i32, ptr %t82.iv
  %t84 = call ptr @global_6insert_P4Treei(ptr %root.1, i32 %t83)
  %t86 = add i32 %i.3.1, 1
  %t78 = icmp slt i32 %t86, 12
  %t82.next = getelementptr i32, ptr %t82.iv, i32 1
  br i1 %t78, label %for.body.3, label %for.end.3

for.end.3:  ; preds = %for.body.3
  br label %for.body.4

for.body.4:  ; preds = %for.end.3, %for.body.4
  %t91.iv = phi i32 [ 0, %for.end.3 ], [ %t91.next, %for.body.4 ]
  %i.4.1 = phi i32 [ 0, %for.end.3 ], [ %t95, %for.body.4 ]
  %root.2 = phi ptr [ %t84, %for.end.3 ], [ %t93, %for.body.4 ]
  %t92 = srem i32 %t91.iv, 101
  %t93 = call ptr @global_6insert_P4Treei(ptr %root.2, i32 %t92)
  %t95 = add i32 %i.4.1, 1
  %t88 = icmp slt i32 %t95, 40
  %t91.next = add i32 %t91.iv, 37
  br i1 %t88, label %for.body.4, label %for.end.4

for.end.4:  ; preds = %for.body.4
  %t96 = getelementptr inbounds [23 x i8], ptr @.str.3, i64 0, i64 0
  %t98 = call i32 @global_5depth_P4Tree(ptr %t93)
  %t100 = call i32 @global_4walk_P4Treei(ptr %t93, i32 0)
  %t101 = call i32 (ptr, ...) @printf(ptr %t96, i32 %t98, i32 %t100)
  call void @global_8freeTree_P4Tree(ptr %t93)
  ret i32 0
}

define void @class_5Stack_C_v(ptr %this) {
entry:
  %t1 = getelementptr inbounds %class.Stack, ptr %this, i32 0, i32 1
  store i32 0, ptr %t1
  ret void
}

define void @class_5Stack_4push_i(ptr %this, i32 %v) {
entry:
  %t2 = getelementptr inbounds %class.Stack, ptr %this, i32 0, i32 0
  %t3 = getelementptr inbounds [64 x i32], ptr %t2, i64 0, i64 0
  %t4 = getelementptr inbounds %class.Stack, ptr %this, i32 0, i32 1
  %t5 = load i32, ptr %t4
  %t6 = getelementptr inbounds i32, ptr %t3, i32 %t5
  store i32 %v, ptr %t6
  %t8 = load i32, ptr %t4
  %t9 = add i32 %t8, 1
  store i32 %t9, ptr %t4
  ret void
}

define i32 @class_5Stack_3pop_v(ptr %this) {
entry:
  %t1 = getelementptr inbounds %class.Stack, ptr %this, i32 0, i32 1
  %t2 = load i32, ptr %t1
  %t3 = sub i32 %t2, 1
  store i32 %t3, ptr %t1
  %t4 = getelementptr inbounds %class.Stack, ptr %this, i32 0, i32 0
  %t5 = getelementptr inbounds [64 x i32], ptr %t4, i64 0, i64 0
  %t8 = getelementptr inbounds i32, ptr %t5, i32 %t3
  %t9 = load i32, ptr %t8
  ret i32 %t9
}

define i1 @class_5Stack_5empty_v(ptr %this) {
entry:
  %t1 = getelementptr inbounds %class.Stack, ptr %this, i32 0, i32 1
  %t2 = load i32, ptr %t1
  %t3 = icmp eq i32 %t2, 0
  ret i1 %t3
}

define void @class_4Ring_C_v(ptr %this) {
entry:
  %t1 = getelementptr inbounds %class.Ring, ptr %this, i32 0, i32 1
  store i32 0, ptr %t1
  %t2 = getelementptr inbounds %class.Ring, ptr %this, i32 0, i32 2
  store i32 0, ptr %t2
  ret void
}

define i1 @class_4Ring_3put_i(ptr %this, i32 %v) {
entry:
  %t1 = getelementptr inbounds %class.Ring, ptr %this, i32 0, i32 2
  %t2 = load i32, ptr %t1
  %t3 = icmp eq i32 %t2, 8
  br i1 %t3, label %if.then, label %if.end

if.then:  ; preds = %entry
  ret i1 false

if.end:  ; preds = %entry
  %t5 = getelementptr inbounds %class.Ring, ptr %this, i32 0, i32 0
  %t6 = getelementptr inbounds [8 x i32], ptr %t5, i64 0, i64 0
  %t7 = getelementptr inbounds %class.Ring, ptr %this, i32 0, i32 1
  %t8 = load i32, ptr %t7
  %t10 = load i32, ptr %t1
  %t11 = add i32 %t8, %t10
  %t12 = srem i32 %t11, 8
  %t13 = getelementptr inbounds i32, ptr %t6, i32 %t12
  store i32 %v, ptr %t13
  %t15 = load i32, ptr %t1
  %t16 = add i32 %t15, 1
  store i32 %t16, ptr %t1
  ret i1 true
}

define i32 @class_4Ring_4take_v(ptr %this) {
entry:
  %t1 = getelementptr inbounds %class.Ring, ptr %this, i32 0, i32 0
  %t2 = getelementptr inbounds [8 x i32], ptr %t1, i64 0, i64 0
  %t3 = getelementptr inbounds %class.Ring, ptr %this, i32 0, i32 1
  %t4 = load i32, ptr %t3
  %t5 = getelementptr inbounds i32, ptr %t2, i32 %t4
  %t6 = load i32, ptr %t5
  %t9 = add i32 %t4, 1
  %t10 = srem i32 %t9, 8
  store i32 %t10, ptr %t3
  %t12 = getelementptr inbounds %class.Ring, ptr %this, i32 0, i32 2
  %t13 = load i32, ptr %t12
  %t14 = sub i32 %t13, 1
  store i32 %t14, ptr %t12
  ret i32 %t6
}

declare ptr @malloc(i64)

declare void @free(ptr)
