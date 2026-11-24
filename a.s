
	.text
	.globl	global_9pushFront_P4Nodei
	.p2align	4
	.type	global_9pushFront_P4Nodei,@function
global_9pushFront_P4Nodei:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$32, %rsp
	movq	%rbx, -24(%rbp)
	movq	%r12, -32(%rbp)
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
.LBB0_entry:
	xorl	%eax, %eax
	leaq	16(%rax), %rax
	movq	%rax, %rbx
	movq	%rax, %rbx
	movq	%rbx, %rdi
	callq	malloc@PLT
	movq	%rax, %rbx
	movq	%rax, %r12
	movl	-16(%rbp), %eax
	movl	%eax, (%r12)
	movq	%rbx, %rax
	leaq	8(%rax), %rax
	movq	%rax, %r12
	movq	-8(%rbp), %rax
	movq	%rax, (%r12)
	movq	%rbx, %rax
.Lret0:
	movq	-24(%rbp), %rbx
	movq	-32(%rbp), %r12
	leave
	ret
	.size	global_9pushFront_P4Nodei, .-global_9pushFront_P4Nodei

	.text
	.globl	global_7reverse_P4Node
	.p2align	4
	.type	global_7reverse_P4Node,@function
global_7reverse_P4Node:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$80, %rsp
	movq	%rbx, -56(%rbp)
	movq	%r12, -64(%rbp)
	movq	%r13, -72(%rbp)
	movq	%r14, -80(%rbp)
	movq	%rdi, -8(%rbp)
.LBB1_entry:
	movq	$0, -16(%rbp)
	movq	-8(%rbp), %rax
	movq	%rax, -24(%rbp)
	movq	$0, -32(%rbp)
	movq	-8(%rbp), %rax
	testq	%rax, %rax
	je	.LBB1_while.end
.LBB1_while.body:
	movq	-16(%rbp), %rax
	movq	%rax, %rbx
	movq	-24(%rbp), %rax
	movq	%rax, %r12
	leaq	8(%rax), %rax
	movq	%rax, %r13
	movq	(%r13), %rax
	movq	%rax, %r14
	movq	%rbx, (%r13)
	movq	%r12, -16(%rbp)
	movq	%r14, -24(%rbp)
	movq	%r12, -40(%rbp)
	movq	%r14, -48(%rbp)
	movq	%r14, %rax
	testq	%rax, %rax
	jne	.LBB1_while.body
.LBB1_while.end.loopexit.1:
	movq	-40(%rbp), %rax
	movq	%rax, -32(%rbp)
.LBB1_while.end:
	movq	-32(%rbp), %rax
	movq	%rax, %rbx
.Lret1:
	movq	-56(%rbp), %rbx
	movq	-64(%rbp), %r12
	movq	-72(%rbp), %r13
	movq	-80(%rbp), %r14
	leave
	ret
	.size	global_7reverse_P4Node, .-global_7reverse_P4Node

	.text
	.globl	global_7sumList_P4Node
	.p2align	4
	.type	global_7sumList_P4Node,@function
global_7sumList_P4Node:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$96, %rsp
	movq	%rbx, -72(%rbp)
	movq	%r12, -80(%rbp)
	movq	%r13, -88(%rbp)
	movq	%r14, -96(%rbp)
	movq	%rdi, -8(%rbp)
.LBB2_entry:
	movl	$1, -16(%rbp)
	movl	$0, -24(%rbp)
	movq	-8(%rbp), %rax
	movq	%rax, -32(%rbp)
	movl	$0, -40(%rbp)
	movq	-8(%rbp), %rax
	testq	%rax, %rax
	je	.LBB2_while.end
.LBB2_while.body:
	movl	-16(%rbp), %eax
	movq	%rax, %rbx
	movl	-24(%rbp), %eax
	movq	%rax, %r12
	movq	-32(%rbp), %rax
	movq	%rax, %r13
	movq	%rax, %r14
	movl	(%r14), %eax
	movq	%rax, %r14
	movl	%r14d, %eax
	imull	%ebx, %eax
	movq	%rax, %r14
	movl	%r12d, %eax
	addl	%r14d, %eax
	movq	%rax, %r12
	movl	%ebx, %eax
	incl	%eax
	movq	%rax, %rbx
	movq	%r13, %rax
	leaq	8(%rax), %rax
	movq	%rax, %r13
	movq	(%r13), %rax
	movq	%rax, %r13
	movl	%ebx, -16(%rbp)
	movl	%r12d, -24(%rbp)
	movq	%r13, -32(%rbp)
	movq	%rbx, -48(%rbp)
	movq	%r12, -56(%rbp)
	movq	%r13, -64(%rbp)
	movq	%r13, %rax
	testq	%rax, %rax
	jne	.LBB2_while.body
.LBB2_while.end.loopexit.1:
	movl	-56(%rbp), %eax
	movl	%eax, -40(%rbp)
.LBB2_while.end:
	movl	-40(%rbp), %eax
	movq	%rax, %rbx
	movl	%ebx, %eax
.Lret2:
	movq	-72(%rbp), %rbx
	movq	-80(%rbp), %r12
	movq	-88(%rbp), %r13
	movq	-96(%rbp), %r14
	leave
	ret
	.size	global_7sumList_P4Node, .-global_7sumList_P4Node

	.text
	.globl	global_8freeList_P4Node
	.p2align	4
	.type	global_8freeList_P4Node,@function
global_8freeList_P4Node:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$48, %rsp
	movq	%rbx, -32(%rbp)
	movq	%r12, -40(%rbp)
	movq	%rdi, -8(%rbp)
.LBB3_entry:
	movq	-8(%rbp), %rax
	movq	%rax, -16(%rbp)
	movq	-8(%rbp), %rax
	testq	%rax, %rax
	je	.LBB3_while.end
.LBB3_while.body:
	movq	-16(%rbp), %rax
	movq	%rax, %rbx
	leaq	8(%rax), %rax
	movq	%rax, %r12
	movq	(%r12), %rax
	movq	%rax, %r12
	movq	%rbx, %rdi
	callq	free@PLT
	movq	%r12, -16(%rbp)
	movq	%r12, -24(%rbp)
	movq	%r12, %rax
	testq	%rax, %rax
	jne	.LBB3_while.body
.LBB3_while.end:
.Lret3:
	movq	-32(%rbp), %rbx
	movq	-40(%rbp), %r12
	leave
	ret
	.size	global_8freeList_P4Node, .-global_8freeList_P4Node

	.text
	.globl	global_6insert_P4Treei
	.p2align	4
	.type	global_6insert_P4Treei,@function
global_6insert_P4Treei:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$48, %rsp
	movq	%rbx, -32(%rbp)
	movq	%r12, -40(%rbp)
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
.LBB4_entry:
	movq	-8(%rbp), %rax
	testq	%rax, %rax
	jne	.LBB4_if.end
.LBB4_if.then:
	xorl	%eax, %eax
	leaq	24(%rax), %rax
	movq	%rax, %rbx
	movq	%rax, %rbx
	movq	%rbx, %rdi
	callq	malloc@PLT
	movq	%rax, %rbx
	movq	%rax, %r12
	movl	-16(%rbp), %eax
	movl	%eax, (%r12)
	movq	%rbx, %rax
	leaq	4(%rax), %rax
	movq	%rax, %r12
	movl	$1, (%r12)
	movq	%rbx, %rax
	leaq	8(%rax), %rax
	movq	%rax, %r12
	movq	$0, (%r12)
	movq	%rbx, %rax
	leaq	16(%rax), %rax
	movq	%rax, %r12
	movq	$0, (%r12)
	movq	%rbx, %rax
	jmp	.Lret4
.LBB4_if.end:
	movq	-8(%rbp), %rax
	movq	%rax, %rbx
	movl	(%rbx), %eax
	movq	%rax, %r12
	movq	%rbx, -24(%rbp)
	movl	-16(%rbp), %eax
	cmpl	%r12d, %eax
	jne	.LBB4_if.else
.LBB4_if.then.1:
	movq	-8(%rbp), %rax
	leaq	4(%rax), %rax
	movq	%rax, %rbx
	movl	(%rbx), %eax
	movq	%rax, %r12
	movl	%r12d, %eax
	incl	%eax
	movq	%rax, %r12
	movl	%r12d, (%rbx)
	jmp	.LBB4_if.end.1
.LBB4_if.else:
	movq	-24(%rbp), %r11
	movl	(%r11), %eax
	movq	%rax, %rbx
	movl	-16(%rbp), %eax
	cmpl	%ebx, %eax
	jge	.LBB4_if.else.1
.LBB4_if.then.2:
	movq	-8(%rbp), %rax
	leaq	8(%rax), %rax
	movq	%rax, %rbx
	movq	(%rbx), %rax
	movq	%rax, %r12
	movq	%r12, %rdi
	movl	-16(%rbp), %esi
	callq	global_6insert_P4Treei
	movq	%rax, %r12
	movq	%r12, (%rbx)
	jmp	.LBB4_if.end.1
.LBB4_if.else.1:
	movq	-8(%rbp), %rax
	leaq	16(%rax), %rax
	movq	%rax, %rbx
	movq	(%rbx), %rax
	movq	%rax, %r12
	movq	%r12, %rdi
	movl	-16(%rbp), %esi
	callq	global_6insert_P4Treei
	movq	%rax, %r12
	movq	%r12, (%rbx)
.LBB4_if.end.1:
	movq	-8(%rbp), %rax
.Lret4:
	movq	-32(%rbp), %rbx
	movq	-40(%rbp), %r12
	leave
	ret
	.size	global_6insert_P4Treei, .-global_6insert_P4Treei

	.text
	.globl	global_5depth_P4Tree
	.p2align	4
	.type	global_5depth_P4Tree,@function
global_5depth_P4Tree:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$48, %rsp
	movq	%rbx, -32(%rbp)
	movq	%r12, -40(%rbp)
	movq	%rdi, -16(%rbp)
.LBB5_entry:
	movq	-16(%rbp), %rax
	testq	%rax, %rax
	jne	.LBB5_if.end
.LBB5_if.then:
	xorl	%eax, %eax
	jmp	.Lret5
.LBB5_if.end:
	movq	-16(%rbp), %rax
	leaq	8(%rax), %rax
	movq	%rax, %rbx
	movq	(%rbx), %rax
	movq	%rax, %rbx
	movq	%rbx, %rdi
	callq	global_5depth_P4Tree
	movq	%rax, %rbx
	movl	%ebx, -4(%rbp)
	movq	-16(%rbp), %rax
	leaq	16(%rax), %rax
	movq	%rax, %rbx
	movq	(%rbx), %rax
	movq	%rax, %rbx
	movq	%rbx, %rdi
	callq	global_5depth_P4Tree
	movq	%rax, %rbx
	movl	%ebx, -8(%rbp)
	movl	-4(%rbp), %eax
	movq	%rax, %r12
	leaq	-4(%rbp), %rax
	movq	%rax, -24(%rbp)
	movl	%r12d, %eax
	cmpl	%ebx, %eax
	jg	.LBB5_cond.end
.LBB5_cond.false:
	leaq	-8(%rbp), %rax
	movq	%rax, -24(%rbp)
.LBB5_cond.end:
	movq	-24(%rbp), %rax
	movq	%rax, %rbx
	movl	(%rbx), %eax
	movq	%rax, %rbx
	movl	$1, %eax
	addl	%ebx, %eax
	movq	%rax, %rbx
	movl	%ebx, %eax
.Lret5:
	movq	-32(%rbp), %rbx
	movq	-40(%rbp), %r12
	leave
	ret
	.size	global_5depth_P4Tree, .-global_5depth_P4Tree

	.text
	.globl	global_4walk_P4Treei
	.p2align	4
	.type	global_4walk_P4Treei,@function
global_4walk_P4Treei:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$48, %rsp
	movq	%rbx, -24(%rbp)
	movq	%r12, -32(%rbp)
	movq	%r13, -40(%rbp)
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
.LBB6_entry:
	movq	-8(%rbp), %rax
	testq	%rax, %rax
	jne	.LBB6_if.end
.LBB6_if.then:
	movl	-16(%rbp), %eax
	jmp	.Lret6
.LBB6_if.end:
	movq	-8(%rbp), %rax
	leaq	8(%rax), %rax
	movq	%rax, %rbx
	movq	(%rbx), %rax
	movq	%rax, %rbx
	movq	%rbx, %rdi
	movl	-16(%rbp), %esi
	callq	global_4walk_P4Treei
	movq	%rax, %rbx
	movl	%ebx, %eax
	imull	$7, %eax
	movq	%rax, %rbx
	movq	-8(%rbp), %rax
	movq	%rax, %r12
	movl	(%r12), %eax
	movq	%rax, %r12
	movq	-8(%rbp), %rax
	leaq	4(%rax), %rax
	movq	%rax, %r13
	movl	(%r13), %eax
	movq	%rax, %r13
	movl	%r12d, %eax
	imull	%r13d, %eax
	movq	%rax, %r12
	movl	%ebx, %eax
	addl	%r12d, %eax
	movq	%rax, %rbx
	movl	%ebx, %eax
	movl	$1000003, %ecx
	cltd
	idivl	%ecx
	movq	%rdx, %rbx
	movq	-8(%rbp), %rax
	leaq	16(%rax), %rax
	movq	%rax, %r12
	movq	(%r12), %rax
	movq	%rax, %r12
	movq	%r12, %rdi
	movl	%ebx, %esi
	callq	global_4walk_P4Treei
	movq	%rax, %rbx
	movl	%ebx, %eax
.Lret6:
	movq	-24(%rbp), %rbx
	movq	-32(%rbp), %r12
	movq	-40(%rbp), %r13
	leave
	ret
	.size	global_4walk_P4Treei, .-global_4walk_P4Treei

	.text
	.globl	global_8freeTree_P4Tree
	.p2align	4
	.type	global_8freeTree_P4Tree,@function
global_8freeTree_P4Tree:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$16, %rsp
	movq	%rbx, -16(%rbp)
	movq	%rdi, -8(%rbp)
.LBB7_entry:
	movq	-8(%rbp), %rax
	testq	%rax, %rax
	jne	.LBB7_if.end
.LBB7_if.then:
	jmp	.Lret7
.LBB7_if.end:
	movq	-8(%rbp), %rax
	leaq	8(%rax), %rax
	movq	%rax, %rbx
	movq	(%rbx), %rax
	movq	%rax, %rbx
	movq	%rbx, %rdi
	callq	global_8freeTree_P4Tree
	movq	-8(%rbp), %rax
	leaq	16(%rax), %rax
	movq	%rax, %rbx
	movq	(%rbx), %rax
	movq	%rax, %rbx
	movq	%rbx, %rdi
	callq	global_8freeTree_P4Tree
	movq	-8(%rbp), %rdi
	callq	free@PLT
.Lret7:
	movq	-16(%rbp), %rbx
	leave
	ret
	.size	global_8freeTree_P4Tree, .-global_8freeTree_P4Tree

	.text
	.globl	main
	.p2align	4
	.type	main,@function
main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$976, %rsp
	movq	%rbx, -936(%rbp)
	movq	%r12, -944(%rbp)
	movq	%r13, -952(%rbp)
	movq	%r14, -960(%rbp)
	movq	%r15, -968(%rbp)
.LBB8_entry:
	xorl	%eax, %eax
	leaq	16(%rax), %rax
	movq	%rax, %rbx
	movq	%rax, %rbx
	movl	$1, -360(%rbp)
	movq	$0, -368(%rbp)
	movq	%rbx, -376(%rbp)
.LBB8_for.body:
	movl	-360(%rbp), %eax
	movq	%rax, %rbx
	movq	-368(%rbp), %rax
	movq	%rax, %r12
	movl	%ebx, %eax
	imull	%ebx, %eax
	movq	%rax, %r13
	movl	%r13d, %eax
	movl	$37, %ecx
	cltd
	idivl	%ecx
	movq	%rdx, %r13
	movq	-376(%rbp), %rdi
	callq	malloc@PLT
	movq	%rax, %r14
	movq	%rax, %r15
	movl	%r13d, (%r15)
	movq	%r14, %rax
	leaq	8(%rax), %rax
	movq	%rax, %r13
	movq	%r12, (%r13)
	movl	%ebx, %eax
	incl	%eax
	movq	%rax, %rbx
	movl	%ebx, -360(%rbp)
	movq	%r14, -368(%rbp)
	movq	%rbx, -384(%rbp)
	movq	%r14, -392(%rbp)
	movl	%ebx, %eax
	cmpl	$50, %eax
	jle	.LBB8_for.body
.LBB8_for.end:
	movq	-392(%rbp), %rax
	testq	%rax, %rax
	setne	%al
	movzbl	%al, %eax
	movq	%rax, %rbx
	movl	$1, -400(%rbp)
	movl	$0, -408(%rbp)
	movq	-392(%rbp), %rax
	movq	%rax, -416(%rbp)
	movl	$0, -424(%rbp)
	movq	%rbx, -432(%rbp)
	testb	$1, %bl
	je	.LBB8_while.end.2
.LBB8_while.body.2:
	movl	-400(%rbp), %eax
	movq	%rax, %rbx
	movl	-408(%rbp), %eax
	movq	%rax, %r12
	movq	-416(%rbp), %rax
	movq	%rax, %r13
	movq	%rax, %r14
	movl	(%r14), %eax
	movq	%rax, %r14
	movl	%r14d, %eax
	imull	%ebx, %eax
	movq	%rax, %r14
	movl	%r12d, %eax
	addl	%r14d, %eax
	movq	%rax, %r12
	movl	%ebx, %eax
	incl	%eax
	movq	%rax, %rbx
	movq	%r13, %rax
	leaq	8(%rax), %rax
	movq	%rax, %r13
	movq	(%r13), %rax
	movq	%rax, %r13
	movl	%ebx, -400(%rbp)
	movl	%r12d, -408(%rbp)
	movq	%r13, -416(%rbp)
	movq	%rbx, -440(%rbp)
	movq	%r12, -448(%rbp)
	movq	%r13, -456(%rbp)
	movq	%r13, %rax
	testq	%rax, %rax
	jne	.LBB8_while.body.2
.LBB8_while.end.2.loopexit:
	movl	-448(%rbp), %eax
	movl	%eax, -424(%rbp)
.LBB8_while.end.2:
	movl	-424(%rbp), %eax
	movq	%rax, %rbx
	movq	$0, -464(%rbp)
	movq	-392(%rbp), %rax
	movq	%rax, -472(%rbp)
	movq	$0, -480(%rbp)
	movq	%rbx, -488(%rbp)
	testb	$1, -432(%rbp)
	je	.LBB8_while.end.3
.LBB8_while.body.3:
	movq	-464(%rbp), %rax
	movq	%rax, %rbx
	movq	-472(%rbp), %rax
	movq	%rax, %r12
	leaq	8(%rax), %rax
	movq	%rax, %r13
	movq	(%r13), %rax
	movq	%rax, %r14
	movq	%rbx, (%r13)
	movq	%r12, -464(%rbp)
	movq	%r14, -472(%rbp)
	movq	%r12, -496(%rbp)
	movq	%r14, -504(%rbp)
	movq	%r14, %rax
	testq	%rax, %rax
	jne	.LBB8_while.body.3
.LBB8_while.end.3.loopexit:
	movq	-496(%rbp), %rax
	movq	%rax, -480(%rbp)
.LBB8_while.end.3:
	movq	-480(%rbp), %rax
	movq	%rax, %rbx
	testq	%rax, %rax
	setne	%al
	movzbl	%al, %eax
	movq	%rax, %r12
	movl	$1, -512(%rbp)
	movl	$0, -520(%rbp)
	movq	%rbx, -528(%rbp)
	movl	$0, -536(%rbp)
	movq	%rbx, -544(%rbp)
	movq	%r12, -552(%rbp)
	testb	$1, %r12b
	je	.LBB8_while.end.4
.LBB8_while.body.4:
	movl	-512(%rbp), %eax
	movq	%rax, %rbx
	movl	-520(%rbp), %eax
	movq	%rax, %r12
	movq	-528(%rbp), %rax
	movq	%rax, %r13
	movq	%rax, %r14
	movl	(%r14), %eax
	movq	%rax, %r14
	movl	%r14d, %eax
	imull	%ebx, %eax
	movq	%rax, %r14
	movl	%r12d, %eax
	addl	%r14d, %eax
	movq	%rax, %r12
	movl	%ebx, %eax
	incl	%eax
	movq	%rax, %rbx
	movq	%r13, %rax
	leaq	8(%rax), %rax
	movq	%rax, %r13
	movq	(%r13), %rax
	movq	%rax, %r13
	movl	%ebx, -512(%rbp)
	movl	%r12d, -520(%rbp)
	movq	%r13, -528(%rbp)
	movq	%rbx, -560(%rbp)
	movq	%r12, -568(%rbp)
	movq	%r13, -576(%rbp)
	movq	%r13, %rax
	testq	%rax, %rax
	jne	.LBB8_while.body.4
.LBB8_while.end.4.loopexit:
	movl	-568(%rbp), %eax
	movl	%eax, -536(%rbp)
.LBB8_while.end.4:
	movl	-536(%rbp), %eax
	movq	%rax, %rbx
	leaq	.L.str(%rip), %rax
	movq	%rax, %r12
	movq	-544(%rbp), %rax
	movq	%rax, %r13
	movl	(%r13), %eax
	movq	%rax, %r13
	movq	%r12, %rdi
	movl	-488(%rbp), %esi
	movl	%ebx, %edx
	movl	%r13d, %ecx
	xorl	%eax, %eax
	callq	printf@PLT
	movq	-544(%rbp), %rax
	movq	%rax, -584(%rbp)
	testb	$1, -552(%rbp)
	je	.LBB8_entry.6
.LBB8_while.body.5:
	movq	-584(%rbp), %rax
	movq	%rax, %rbx
	leaq	8(%rax), %rax
	movq	%rax, %r12
	movq	(%r12), %rax
	movq	%rax, %r12
	movq	%rbx, %rdi
	callq	free@PLT
	movq	%r12, -584(%rbp)
	movq	%r12, -592(%rbp)
	movq	%r12, %rax
	testq	%rax, %rax
	jne	.LBB8_while.body.5
.LBB8_entry.6:
	leaq	-260(%rbp), %rax
	leaq	256(%rax), %rax
	movq	%rax, %rbx
	movl	$0, (%rbx)
	leaq	-260(%rbp), %rax
	movq	%rax, %r12
	movq	%rax, %r12
	movl	$0, -600(%rbp)
	movl	$0, -608(%rbp)
	movq	%rbx, -616(%rbp)
	movq	%r12, -624(%rbp)
.LBB8_for.body.1:
	movl	-600(%rbp), %eax
	movq	%rax, %rbx
	movl	-608(%rbp), %eax
	movq	%rax, %r12
	movq	-616(%rbp), %r11
	movl	(%r11), %eax
	movq	%rax, %r13
	movq	-624(%rbp), %rax
	movl	%r13d, %ecx
	movslq	%ecx, %rcx
	imulq	$4, %rcx
	addq	%rcx, %rax
	movq	%rax, %r13
	movl	%ebx, (%r13)
	movq	-616(%rbp), %r11
	movl	(%r11), %eax
	movq	%rax, %r13
	movl	%r13d, %eax
	incl	%eax
	movq	%rax, %r13
	movq	-616(%rbp), %r11
	movl	%r13d, (%r11)
	movl	%r12d, %eax
	incl	%eax
	movq	%rax, %r12
	movl	%r12d, %eax
	cmpl	$20, %eax
	setl	%al
	movzbl	%al, %eax
	movq	%rax, %r13
	movl	%ebx, %eax
	addl	$3, %eax
	movq	%rax, %rbx
	movl	%ebx, -600(%rbp)
	movl	%r12d, -608(%rbp)
	movq	%rbx, -632(%rbp)
	movq	%r12, -640(%rbp)
	testb	$1, %r13b
	jne	.LBB8_for.body.1
.LBB8_for.end.1:
	movl	$0, -648(%rbp)
.LBB8_entry.8:
	movl	-648(%rbp), %eax
	movq	%rax, %rbx
	movq	-616(%rbp), %r11
	movl	(%r11), %eax
	movq	%rax, %r12
	movl	%r12d, %eax
	testl	%eax, %eax
	sete	%al
	movzbl	%al, %eax
	movq	%rax, %r12
	movl	%r12d, %eax
	xorl	$1, %eax
	movq	%rax, %r12
	movq	%rbx, -656(%rbp)
	testb	$1, %r12b
	je	.LBB8_while.end
.LBB8_while.body:
	movl	-656(%rbp), %eax
	movl	$1, %ecx
	shll	%cl, %eax
	movq	%rax, %rbx
	movl	%ebx, %eax
	movl	$10007, %ecx
	cltd
	idivl	%ecx
	movq	%rdx, %rbx
	movq	-616(%rbp), %r11
	movl	(%r11), %eax
	movq	%rax, %r12
	movl	%r12d, %eax
	decl	%eax
	movq	%rax, %r12
	movq	-616(%rbp), %r11
	movl	%r12d, (%r11)
	movq	-624(%rbp), %rax
	movl	%r12d, %ecx
	movslq	%ecx, %rcx
	imulq	$4, %rcx
	addq	%rcx, %rax
	movq	%rax, %r12
	movl	(%r12), %eax
	movq	%rax, %r12
	movl	%ebx, %eax
	addl	%r12d, %eax
	movq	%rax, %rbx
	movl	%ebx, -648(%rbp)
	movq	%rbx, -664(%rbp)
	jmp	.LBB8_entry.8
.LBB8_while.end:
	leaq	.L.str.1(%rip), %rax
	movq	%rax, %rbx
	movq	%rbx, %rdi
	movl	-656(%rbp), %esi
	xorl	%eax, %eax
	callq	printf@PLT
	leaq	-300(%rbp), %rax
	leaq	32(%rax), %rax
	movq	%rax, %rbx
	movl	$0, (%rbx)
	leaq	-300(%rbp), %rax
	leaq	36(%rax), %rax
	movq	%rax, %r12
	movl	$0, (%r12)
	leaq	-300(%rbp), %rax
	movq	%rax, %r13
	movq	%rax, %r13
	movl	$0, -672(%rbp)
	movl	$0, -680(%rbp)
	movl	$0, -688(%rbp)
	movq	%rbx, -696(%rbp)
	movq	%r12, -704(%rbp)
	movq	%r13, -712(%rbp)
.LBB8_for.body.2:
	movl	-672(%rbp), %eax
	movq	%rax, %rbx
	movl	-680(%rbp), %eax
	movq	%rax, %r12
	movl	-688(%rbp), %eax
	movq	%rax, %r13
	movq	-704(%rbp), %r11
	movl	(%r11), %eax
	movq	%rax, %r14
	movb	$0, -720(%rbp)
	movq	%rbx, -728(%rbp)
	movq	%r12, -736(%rbp)
	movq	%r13, -744(%rbp)
	movl	%r14d, %eax
	cmpl	$8, %eax
	je	.LBB8_class_4Ring_3put_i.exit
.LBB8_if.end.2:
	movq	-696(%rbp), %r11
	movl	(%r11), %eax
	movq	%rax, %rbx
	movq	-704(%rbp), %r11
	movl	(%r11), %eax
	movq	%rax, %r12
	movl	%ebx, %eax
	addl	%r12d, %eax
	movq	%rax, %rbx
	movl	%ebx, %eax
	movl	$8, %ecx
	cltd
	idivl	%ecx
	movq	%rdx, %rbx
	movq	-712(%rbp), %rax
	movl	%ebx, %ecx
	movslq	%ecx, %rcx
	imulq	$4, %rcx
	addq	%rcx, %rax
	movq	%rax, %rbx
	movl	-728(%rbp), %eax
	movl	%eax, (%rbx)
	movq	-704(%rbp), %r11
	movl	(%r11), %eax
	movq	%rax, %rbx
	movl	%ebx, %eax
	addl	$1, %eax
	movq	%rax, %rbx
	movq	-704(%rbp), %r11
	movl	%ebx, (%r11)
	movb	$1, -720(%rbp)
.LBB8_class_4Ring_3put_i.exit:
	movzbl	-720(%rbp), %eax
	movq	%rax, %rbx
	movl	%ebx, %eax
	xorl	$1, %eax
	movq	%rax, %rbx
	movl	-744(%rbp), %eax
	movl	%eax, -752(%rbp)
	testb	$1, %bl
	je	.LBB8_if.end
.LBB8_if.then:
	movl	-744(%rbp), %eax
	addl	$1, %eax
	movq	%rax, %rbx
	movl	%ebx, -752(%rbp)
	movq	%rbx, -760(%rbp)
.LBB8_if.end:
	movl	-752(%rbp), %eax
	movq	%rax, %rbx
	movl	-728(%rbp), %eax
	movl	$3, %ecx
	cltd
	idivl	%ecx
	movq	%rdx, %r12
	movl	-736(%rbp), %eax
	movl	%eax, -768(%rbp)
	movq	%rbx, -776(%rbp)
	movl	%r12d, %eax
	cmpl	$2, %eax
	jne	.LBB8_for.inc.2
.LBB8_entry.12:
	movq	-696(%rbp), %r11
	movl	(%r11), %eax
	movq	%rax, %rbx
	movq	-712(%rbp), %rax
	movl	%ebx, %ecx
	movslq	%ecx, %rcx
	imulq	$4, %rcx
	addq	%rcx, %rax
	movq	%rax, %r12
	movl	(%r12), %eax
	movq	%rax, %r12
	movl	%ebx, %eax
	incl	%eax
	movq	%rax, %rbx
	movl	%ebx, %eax
	movl	$8, %ecx
	cltd
	idivl	%ecx
	movq	%rdx, %rbx
	movq	-696(%rbp), %r11
	movl	%ebx, (%r11)
	movq	-704(%rbp), %r11
	movl	(%r11), %eax
	movq	%rax, %rbx
	movl	%ebx, %eax
	decl	%eax
	movq	%rax, %rbx
	movq	-704(%rbp), %r11
	movl	%ebx, (%r11)
	movl	-736(%rbp), %eax
	addl	%r12d, %eax
	movq	%rax, %rbx
	movl	%ebx, -768(%rbp)
	movq	%rbx, -784(%rbp)
.LBB8_for.inc.2:
	movl	-768(%rbp), %eax
	movq	%rax, %rbx
	movl	-728(%rbp), %eax
	incl	%eax
	movq	%rax, %r12
	movl	%r12d, -672(%rbp)
	movl	%ebx, -680(%rbp)
	movl	-776(%rbp), %eax
	movl	%eax, -688(%rbp)
	movq	%rbx, -792(%rbp)
	movq	%r12, -800(%rbp)
	movl	%r12d, %eax
	cmpl	$30, %eax
	jl	.LBB8_for.body.2
.LBB8_while.cond.1.loopexit:
	movq	-704(%rbp), %r11
	movl	(%r11), %eax
	movq	%rax, %rbx
	movl	-792(%rbp), %eax
	movl	%eax, -808(%rbp)
	movl	%ebx, %eax
	testl	%eax, %eax
	jle	.LBB8_while.end.1
.LBB8_entry.13.preheader:
	movl	-792(%rbp), %eax
	movl	%eax, -816(%rbp)
.LBB8_entry.13:
	movl	-816(%rbp), %eax
	movq	%rax, %rbx
	movq	-696(%rbp), %r11
	movl	(%r11), %eax
	movq	%rax, %r12
	movq	-712(%rbp), %rax
	movl	%r12d, %ecx
	movslq	%ecx, %rcx
	imulq	$4, %rcx
	addq	%rcx, %rax
	movq	%rax, %r13
	movl	(%r13), %eax
	movq	%rax, %r13
	movl	%r12d, %eax
	incl	%eax
	movq	%rax, %r12
	movl	%r12d, %eax
	movl	$8, %ecx
	cltd
	idivl	%ecx
	movq	%rdx, %r12
	movq	-696(%rbp), %r11
	movl	%r12d, (%r11)
	movq	-704(%rbp), %r11
	movl	(%r11), %eax
	movq	%rax, %r12
	movl	%r12d, %eax
	subl	$1, %eax
	movq	%rax, %r12
	movq	-704(%rbp), %r11
	movl	%r12d, (%r11)
	movl	%r13d, %eax
	movl	$1, %ecx
	shll	%cl, %eax
	movq	%rax, %r12
	movl	%ebx, %eax
	addl	%r12d, %eax
	movq	%rax, %rbx
	movq	-704(%rbp), %r11
	movl	(%r11), %eax
	movq	%rax, %r12
	movl	%ebx, -816(%rbp)
	movq	%rbx, -824(%rbp)
	movl	%r12d, %eax
	testl	%eax, %eax
	jg	.LBB8_entry.13
.LBB8_while.end.1.loopexit.1:
	movl	-824(%rbp), %eax
	movl	%eax, -808(%rbp)
.LBB8_while.end.1:
	movl	-808(%rbp), %eax
	movq	%rax, %rbx
	leaq	.L.str.2(%rip), %rax
	movq	%rax, %r12
	movq	%r12, %rdi
	movl	-776(%rbp), %esi
	movl	%ebx, %edx
	xorl	%eax, %eax
	callq	printf@PLT
	leaq	-348(%rbp), %rax
	movq	%rax, %rbx
	movl	$50, (%rbx)
	leaq	-348(%rbp), %rax
	leaq	4(%rax), %rax
	movq	%rax, %r12
	movl	$30, (%r12)
	leaq	-348(%rbp), %rax
	leaq	8(%rax), %rax
	movq	%rax, %r12
	movl	$70, (%r12)
	leaq	-348(%rbp), %rax
	leaq	12(%rax), %rax
	movq	%rax, %r12
	movl	$20, (%r12)
	leaq	-348(%rbp), %rax
	leaq	16(%rax), %rax
	movq	%rax, %r12
	movl	$40, (%r12)
	leaq	-348(%rbp), %rax
	leaq	20(%rax), %rax
	movq	%rax, %r12
	movl	$60, (%r12)
	leaq	-348(%rbp), %rax
	leaq	24(%rax), %rax
	movq	%rax, %r12
	movl	$80, (%r12)
	leaq	-348(%rbp), %rax
	leaq	28(%rax), %rax
	movq	%rax, %r12
	movl	$30, (%r12)
	leaq	-348(%rbp), %rax
	leaq	32(%rax), %rax
	movq	%rax, %r12
	movl	$65, (%r12)
	leaq	-348(%rbp), %rax
	leaq	36(%rax), %rax
	movq	%rax, %r12
	movl	$20, (%r12)
	leaq	-348(%rbp), %rax
	leaq	40(%rax), %rax
	movq	%rax, %r12
	movl	$85, (%r12)
	leaq	-348(%rbp), %rax
	leaq	44(%rax), %rax
	movq	%rax, %r12
	movl	$50, (%r12)
	movq	%rbx, %rax
	movq	%rbx, -832(%rbp)
	movl	$0, -840(%rbp)
	movq	$0, -848(%rbp)
	movq	%rbx, -856(%rbp)
.LBB8_for.body.3:
	movq	-832(%rbp), %rax
	movq	%rax, %rbx
	movl	-840(%rbp), %eax
	movq	%rax, %r12
	movq	-848(%rbp), %rax
	movq	%rax, %r13
	movl	(%rbx), %eax
	movq	%rax, %r14
	movq	%r13, %rdi
	movl	%r14d, %esi
	callq	global_6insert_P4Treei
	movq	%rax, %r13
	movl	%r12d, %eax
	incl	%eax
	movq	%rax, %r12
	movl	%r12d, %eax
	cmpl	$12, %eax
	setl	%al
	movzbl	%al, %eax
	movq	%rax, %r14
	movq	%rbx, %rax
	leaq	4(%rax), %rax
	movq	%rax, %rbx
	movq	%rbx, -832(%rbp)
	movl	%r12d, -840(%rbp)
	movq	%r13, -848(%rbp)
	movq	%rbx, -864(%rbp)
	movq	%r12, -872(%rbp)
	movq	%r13, -880(%rbp)
	testb	$1, %r14b
	jne	.LBB8_for.body.3
.LBB8_for.end.3:
	movl	$0, -888(%rbp)
	movl	$0, -896(%rbp)
	movq	-880(%rbp), %rax
	movq	%rax, -904(%rbp)
.LBB8_for.body.4:
	movl	-888(%rbp), %eax
	movq	%rax, %rbx
	movl	-896(%rbp), %eax
	movq	%rax, %r12
	movq	-904(%rbp), %rax
	movq	%rax, %r13
	movl	%ebx, %eax
	movl	$101, %ecx
	cltd
	idivl	%ecx
	movq	%rdx, %r14
	movq	%r13, %rdi
	movl	%r14d, %esi
	callq	global_6insert_P4Treei
	movq	%rax, %r13
	movl	%r12d, %eax
	incl	%eax
	movq	%rax, %r12
	movl	%r12d, %eax
	cmpl	$40, %eax
	setl	%al
	movzbl	%al, %eax
	movq	%rax, %r14
	movl	%ebx, %eax
	addl	$37, %eax
	movq	%rax, %rbx
	movl	%ebx, -888(%rbp)
	movl	%r12d, -896(%rbp)
	movq	%r13, -904(%rbp)
	movq	%rbx, -912(%rbp)
	movq	%r12, -920(%rbp)
	movq	%r13, -928(%rbp)
	testb	$1, %r14b
	jne	.LBB8_for.body.4
.LBB8_for.end.4:
	leaq	.L.str.3(%rip), %rax
	movq	%rax, %rbx
	movq	-928(%rbp), %rdi
	callq	global_5depth_P4Tree
	movq	%rax, %r12
	movq	-928(%rbp), %rdi
	xorl	%esi, %esi
	callq	global_4walk_P4Treei
	movq	%rax, %r13
	movq	%rbx, %rdi
	movl	%r12d, %esi
	movl	%r13d, %edx
	xorl	%eax, %eax
	callq	printf@PLT
	movq	-928(%rbp), %rdi
	callq	global_8freeTree_P4Tree
	xorl	%eax, %eax
.Lret8:
	movq	-936(%rbp), %rbx
	movq	-944(%rbp), %r12
	movq	-952(%rbp), %r13
	movq	-960(%rbp), %r14
	movq	-968(%rbp), %r15
	leave
	ret
	.size	main, .-main

	.text
	.globl	class_5Stack_C_v
	.p2align	4
	.type	class_5Stack_C_v,@function
class_5Stack_C_v:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$16, %rsp
	movq	%rbx, -16(%rbp)
	movq	%rdi, -8(%rbp)
.LBB9_entry:
	movq	-8(%rbp), %rax
	leaq	256(%rax), %rax
	movq	%rax, %rbx
	movl	$0, (%rbx)
.Lret9:
	movq	-16(%rbp), %rbx
	leave
	ret
	.size	class_5Stack_C_v, .-class_5Stack_C_v

	.text
	.globl	class_5Stack_4push_i
	.p2align	4
	.type	class_5Stack_4push_i,@function
class_5Stack_4push_i:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$48, %rsp
	movq	%rbx, -24(%rbp)
	movq	%r12, -32(%rbp)
	movq	%r13, -40(%rbp)
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
.LBB10_entry:
	movq	-8(%rbp), %rax
	movq	%rax, %rbx
	movq	%rax, %rbx
	movq	-8(%rbp), %rax
	leaq	256(%rax), %rax
	movq	%rax, %r12
	movl	(%r12), %eax
	movq	%rax, %r13
	movq	%rbx, %rax
	movl	%r13d, %ecx
	movslq	%ecx, %rcx
	imulq	$4, %rcx
	addq	%rcx, %rax
	movq	%rax, %rbx
	movl	-16(%rbp), %eax
	movl	%eax, (%rbx)
	movl	(%r12), %eax
	movq	%rax, %rbx
	movl	%ebx, %eax
	addl	$1, %eax
	movq	%rax, %rbx
	movl	%ebx, (%r12)
.Lret10:
	movq	-24(%rbp), %rbx
	movq	-32(%rbp), %r12
	movq	-40(%rbp), %r13
	leave
	ret
	.size	class_5Stack_4push_i, .-class_5Stack_4push_i

	.text
	.globl	class_5Stack_3pop_v
	.p2align	4
	.type	class_5Stack_3pop_v,@function
class_5Stack_3pop_v:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$32, %rsp
	movq	%rbx, -16(%rbp)
	movq	%r12, -24(%rbp)
	movq	%rdi, -8(%rbp)
.LBB11_entry:
	movq	-8(%rbp), %rax
	leaq	256(%rax), %rax
	movq	%rax, %rbx
	movl	(%rbx), %eax
	movq	%rax, %r12
	movl	%r12d, %eax
	decl	%eax
	movq	%rax, %r12
	movl	%r12d, (%rbx)
	movq	-8(%rbp), %rax
	movq	%rax, %rbx
	movq	%rax, %rbx
	movl	%r12d, %ecx
	movslq	%ecx, %rcx
	imulq	$4, %rcx
	addq	%rcx, %rax
	movq	%rax, %rbx
	movl	(%rbx), %eax
	movq	%rax, %rbx
	movl	%ebx, %eax
.Lret11:
	movq	-16(%rbp), %rbx
	movq	-24(%rbp), %r12
	leave
	ret
	.size	class_5Stack_3pop_v, .-class_5Stack_3pop_v

	.text
	.globl	class_5Stack_5empty_v
	.p2align	4
	.type	class_5Stack_5empty_v,@function
class_5Stack_5empty_v:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$16, %rsp
	movq	%rbx, -16(%rbp)
	movq	%rdi, -8(%rbp)
.LBB12_entry:
	movq	-8(%rbp), %rax
	leaq	256(%rax), %rax
	movq	%rax, %rbx
	movl	(%rbx), %eax
	movq	%rax, %rbx
	movl	%ebx, %eax
	testl	%eax, %eax
	sete	%al
	movzbl	%al, %eax
	movq	%rax, %rbx
	movl	%ebx, %eax
.Lret12:
	movq	-16(%rbp), %rbx
	leave
	ret
	.size	class_5Stack_5empty_v, .-class_5Stack_5empty_v

	.text
	.globl	class_4Ring_C_v
	.p2align	4
	.type	class_4Ring_C_v,@function
class_4Ring_C_v:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$16, %rsp
	movq	%rbx, -16(%rbp)
	movq	%rdi, -8(%rbp)
.LBB13_entry:
	movq	-8(%rbp), %rax
	leaq	32(%rax), %rax
	movq	%rax, %rbx
	movl	$0, (%rbx)
	movq	-8(%rbp), %rax
	leaq	36(%rax), %rax
	movq	%rax, %rbx
	movl	$0, (%rbx)
.Lret13:
	movq	-16(%rbp), %rbx
	leave
	ret
	.size	class_4Ring_C_v, .-class_4Ring_C_v

	.text
	.globl	class_4Ring_3put_i
	.p2align	4
	.type	class_4Ring_3put_i,@function
class_4Ring_3put_i:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$48, %rsp
	movq	%rbx, -32(%rbp)
	movq	%r12, -40(%rbp)
	movq	%r13, -48(%rbp)
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
.LBB14_entry:
	movq	-8(%rbp), %rax
	leaq	36(%rax), %rax
	movq	%rax, %rbx
	movl	(%rbx), %eax
	movq	%rax, %r12
	movq	%rbx, -24(%rbp)
	movl	%r12d, %eax
	cmpl	$8, %eax
	jne	.LBB14_if.end
.LBB14_if.then:
	xorl	%eax, %eax
	jmp	.Lret14
.LBB14_if.end:
	movq	-8(%rbp), %rax
	movq	%rax, %rbx
	movq	%rax, %rbx
	movq	-8(%rbp), %rax
	leaq	32(%rax), %rax
	movq	%rax, %r12
	movl	(%r12), %eax
	movq	%rax, %r12
	movq	-24(%rbp), %r11
	movl	(%r11), %eax
	movq	%rax, %r13
	movl	%r12d, %eax
	addl	%r13d, %eax
	movq	%rax, %r12
	movl	%r12d, %eax
	movl	$8, %ecx
	cltd
	idivl	%ecx
	movq	%rdx, %r12
	movq	%rbx, %rax
	movl	%r12d, %ecx
	movslq	%ecx, %rcx
	imulq	$4, %rcx
	addq	%rcx, %rax
	movq	%rax, %rbx
	movl	-16(%rbp), %eax
	movl	%eax, (%rbx)
	movq	-24(%rbp), %r11
	movl	(%r11), %eax
	movq	%rax, %rbx
	movl	%ebx, %eax
	addl	$1, %eax
	movq	%rax, %rbx
	movq	-24(%rbp), %r11
	movl	%ebx, (%r11)
	movl	$1, %eax
.Lret14:
	movq	-32(%rbp), %rbx
	movq	-40(%rbp), %r12
	movq	-48(%rbp), %r13
	leave
	ret
	.size	class_4Ring_3put_i, .-class_4Ring_3put_i

	.text
	.globl	class_4Ring_4take_v
	.p2align	4
	.type	class_4Ring_4take_v,@function
class_4Ring_4take_v:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$32, %rsp
	movq	%rbx, -16(%rbp)
	movq	%r12, -24(%rbp)
	movq	%r13, -32(%rbp)
	movq	%rdi, -8(%rbp)
.LBB15_entry:
	movq	-8(%rbp), %rax
	movq	%rax, %rbx
	movq	%rax, %rbx
	movq	-8(%rbp), %rax
	leaq	32(%rax), %rax
	movq	%rax, %r12
	movl	(%r12), %eax
	movq	%rax, %r13
	movq	%rbx, %rax
	movl	%r13d, %ecx
	movslq	%ecx, %rcx
	imulq	$4, %rcx
	addq	%rcx, %rax
	movq	%rax, %rbx
	movl	(%rbx), %eax
	movq	%rax, %rbx
	movl	%r13d, %eax
	incl	%eax
	movq	%rax, %r13
	movl	%r13d, %eax
	movl	$8, %ecx
	cltd
	idivl	%ecx
	movq	%rdx, %r13
	movl	%r13d, (%r12)
	movq	-8(%rbp), %rax
	leaq	36(%rax), %rax
	movq	%rax, %r12
	movl	(%r12), %eax
	movq	%rax, %r13
	movl	%r13d, %eax
	subl	$1, %eax
	movq	%rax, %r13
	movl	%r13d, (%r12)
	movl	%ebx, %eax
.Lret15:
	movq	-16(%rbp), %rbx
	movq	-24(%rbp), %r12
	movq	-32(%rbp), %r13
	leave
	ret
	.size	class_4Ring_4take_v, .-class_4Ring_4take_v

	.section	.rodata
	.p2align	0
	.type	.L.str,@object
.L.str:
	.ascii	"list %d %d first %d\012\000"
	.size	.L.str, 21

	.section	.rodata
	.p2align	0
	.type	.L.str.1,@object
.L.str.1:
	.ascii	"stack %d\012\000"
	.size	.L.str.1, 10

	.section	.rodata
	.p2align	0
	.type	.L.str.2,@object
.L.str.2:
	.ascii	"ring %d %d\012\000"
	.size	.L.str.2, 12

	.section	.rodata
	.p2align	0
	.type	.L.str.3,@object
.L.str.3:
	.ascii	"tree depth %d walk %d\012\000"
	.size	.L.str.3, 23

	.section	.note.GNU-stack,"",@progbits
