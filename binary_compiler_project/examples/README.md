# 示例代码 / Example Code

## 目录结构 / Directory Structure

```
examples/
├── basic/              # 基础示例 / Basic Examples
│   ├── hello.lsp           # Hello World
│   ├── arithmetic.lsp      # 算术运算
│   ├── functions.lsp       # 函数定义
│   └── control_flow.lsp    # 控制流
├── advanced/           # 高级示例 / Advanced Examples
│   ├── recursion.lsp       # 递归算法
│   ├── closures.lsp        # 闭包和lambda
│   ├── data_structures.lsp # 数据结构
│   └── file_processing.lsp # 文件处理
└── benchmarks/         # 性能测试 / Performance Benchmarks
    ├── fibonacci.lsp       # 斐波那契数列
    ├── quicksort.lsp       # 快速排序
    ├── matrix_multiply.lsp # 矩阵乘法
    └── string_processing.lsp # 字符串处理
```

## 基础示例 / Basic Examples

### Hello World

```lisp
;; examples/basic/hello.lsp
;; 最简单的Hello World程序

(println "Hello, World!")
(println "Welcome to newLISP Binary Compiler!")

;; 编译命令 / Compilation command:
;; ./nlcc hello.lsp -o hello
;; ./hello
```

### 算术运算 / Arithmetic Operations

```lisp
;; examples/basic/arithmetic.lsp
;; 基础算术运算示例

;; 基本运算 / Basic operations
(println "Addition: " (+ 10 20))
(println "Subtraction: " (- 30 15))
(println "Multiplication: " (* 6 7))
(println "Division: " (/ 100 4))

;; 复合运算 / Compound operations
(println "Complex: " (+ (* 3 4) (/ 20 5)))

;; 浮点运算 / Floating point operations
(println "Float division: " (div 22 7))
(println "Square root: " (sqrt 16))
(println "Power: " (pow 2 8))

;; 编译提示 / Compilation hints:
;; 这个示例展示了数值计算的编译优化潜力
;; This example demonstrates compilation optimization potential for numeric computations
```

### 函数定义 / Function Definition

```lisp
;; examples/basic/functions.lsp
;; 函数定义和调用示例

;; 简单函数 / Simple function
(define (square x)
  (* x x))

;; 多参数函数 / Multi-parameter function
(define (add-three a b c)
  (+ a b c))

;; 递归函数 / Recursive function
(define (factorial n)
  (if (= n 0)
      1
      (* n (factorial (- n 1)))))

;; 高阶函数 / Higher-order function
(define (apply-twice f x)
  (f (f x)))

;; 测试函数 / Test functions
(println "Square of 5: " (square 5))
(println "Sum of 1,2,3: " (add-three 1 2 3))
(println "Factorial of 5: " (factorial 5))
(println "Apply square twice to 3: " (apply-twice square 3))

;; 性能提示 / Performance hint:
;; 函数调用是编译优化的重点，特别是递归函数
;; Function calls are key targets for compilation optimization, especially recursive functions
```

### 控制流 / Control Flow

```lisp
;; examples/basic/control_flow.lsp
;; 控制流语句示例

;; 条件语句 / Conditional statements
(define (check-number n)
  (cond
    ((> n 0) "positive")
    ((< n 0) "negative")
    (true "zero")))

;; 循环结构 / Loop structures
(define (count-down n)
  (while (> n 0)
    (println n)
    (dec n)))

;; 列表遍历 / List iteration
(define (sum-list lst)
  (let ((total 0))
    (dolist (item lst)
      (inc total item))
    total))

;; 测试控制流 / Test control flow
(println "Number check: " (check-number -5))
(println "Counting down from 5:")
(count-down 5)
(println "Sum of list: " (sum-list '(1 2 3 4 5)))

;; 编译优化提示 / Compilation optimization hint:
;; 循环和条件语句是JIT编译的主要优化目标
;; Loops and conditionals are primary targets for JIT compilation optimization
```

## 高级示例 / Advanced Examples

### 递归算法 / Recursive Algorithms

```lisp
;; examples/advanced/recursion.lsp
;; 递归算法示例，展示编译优化效果

;; 斐波那契数列 (递归版本) / Fibonacci sequence (recursive version)
(define (fib n)
  (if (< n 2)
      n
      (+ (fib (- n 1)) (fib (- n 2)))))

;; 斐波那契数列 (尾递归版本) / Fibonacci sequence (tail recursive version)
(define (fib-tail n)
  (define (fib-helper a b count)
    (if (= count 0)
        a
        (fib-helper b (+ a b) (- count 1))))
  (fib-helper 0 1 n))

;; 快速幂算法 / Fast exponentiation
(define (fast-power base exp)
  (cond
    ((= exp 0) 1)
    ((even? exp) (let ((half (fast-power base (/ exp 2))))
                   (* half half)))
    (true (* base (fast-power base (- exp 1))))))

;; 汉诺塔问题 / Tower of Hanoi
(define (hanoi n from to aux)
  (if (= n 1)
      (println "Move disk from " from " to " to)
      (begin
        (hanoi (- n 1) from aux to)
        (println "Move disk from " from " to " to)
        (hanoi (- n 1) aux to from))))

;; 性能测试 / Performance testing
(println "Fibonacci(30) recursive: " (fib 30))
(println "Fibonacci(30) tail recursive: " (fib-tail 30))
(println "2^20 = " (fast-power 2 20))
(println "Hanoi tower with 3 disks:")
(hanoi 3 "A" "C" "B")

;; 编译优化说明 / Compilation optimization notes:
;; 递归函数是编译器优化的重点，特别是尾递归优化
;; Recursive functions are key targets for compiler optimization, especially tail call optimization
```

### 闭包和Lambda / Closures and Lambda

```lisp
;; examples/advanced/closures.lsp
;; 闭包和lambda表达式示例

;; 创建闭包 / Create closure
(define (make-counter start)
  (let ((count start))
    (lambda ()
      (inc count))))

;; 高阶函数 / Higher-order functions
(define (map-function f lst)
  (if (empty? lst)
      '()
      (cons (f (first lst))
            (map-function f (rest lst)))))

(define (filter-function pred lst)
  (if (empty? lst)
      '()
      (if (pred (first lst))
          (cons (first lst) (filter-function pred (rest lst)))
          (filter-function pred (rest lst)))))

;; 函数组合 / Function composition
(define (compose f g)
  (lambda (x) (f (g x))))

;; 柯里化 / Currying
(define (curry-add x)
  (lambda (y) (+ x y)))

;; 测试闭包和lambda / Test closures and lambda
(set 'counter1 (make-counter 0))
(set 'counter2 (make-counter 100))

(println "Counter1: " (counter1) " " (counter1) " " (counter1))
(println "Counter2: " (counter2) " " (counter2))

(set 'numbers '(1 2 3 4 5))
(println "Squared: " (map-function (lambda (x) (* x x)) numbers))
(println "Even numbers: " (filter-function even? numbers))

(set 'add-one (curry-add 1))
(println "Add one to 5: " (add-one 5))

(set 'square-then-double (compose (lambda (x) (* x 2)) (lambda (x) (* x x))))
(println "Square then double 3: " (square-then-double 3))

;; 编译挑战 / Compilation challenges:
;; 闭包和lambda需要特殊的编译处理，涉及环境捕获和函数对象
;; Closures and lambdas require special compilation handling, involving environment capture and function objects
```

## 性能基准测试 / Performance Benchmarks

### 斐波那契基准 / Fibonacci Benchmark

```lisp
;; examples/benchmarks/fibonacci.lsp
;; 斐波那契数列性能基准测试

;; 递归版本 (性能较差，但测试编译器优化效果) / Recursive version (poor performance, but tests compiler optimization)
(define (fib-recursive n)
  (if (< n 2)
      n
      (+ (fib-recursive (- n 1)) (fib-recursive (- n 2)))))

;; 迭代版本 (性能较好) / Iterative version (better performance)
(define (fib-iterative n)
  (let ((a 0) (b 1) (i 0))
    (while (< i n)
      (let ((temp a))
        (set 'a b)
        (set 'b (+ temp b))
        (inc i)))
    a))

;; 基准测试函数 / Benchmark function
(define (benchmark-fibonacci)
  (println "=== Fibonacci Benchmark ===")
  
  ;; 测试递归版本 / Test recursive version
  (set 'start-time (time-of-day))
  (set 'result1 (fib-recursive 35))
  (set 'recursive-time (- (time-of-day) start-time))
  
  ;; 测试迭代版本 / Test iterative version
  (set 'start-time (time-of-day))
  (set 'result2 (fib-iterative 35))
  (set 'iterative-time (- (time-of-day) start-time))
  
  (println "Recursive fib(35): " result1 " Time: " recursive-time "ms")
  (println "Iterative fib(35): " result2 " Time: " iterative-time "ms")
  (println "Speedup ratio: " (div recursive-time iterative-time)))

;; 运行基准测试 / Run benchmark
(benchmark-fibonacci)

;; 编译性能期望 / Compilation performance expectations:
;; 递归版本: 期望5-10倍性能提升 / Recursive version: expect 5-10x performance improvement
;; 迭代版本: 期望2-3倍性能提升 / Iterative version: expect 2-3x performance improvement
```

### 快速排序基准 / Quick Sort Benchmark

```lisp
;; examples/benchmarks/quicksort.lsp
;; 快速排序算法性能基准测试

;; 快速排序实现 / Quick sort implementation
(define (quicksort lst)
  (if (or (empty? lst) (= (length lst) 1))
      lst
      (let ((pivot (first lst))
            (rest-list (rest lst)))
        (append
          (quicksort (filter (lambda (x) (< x pivot)) rest-list))
          (list pivot)
          (quicksort (filter (lambda (x) (>= x pivot)) rest-list))))))

;; 生成随机数列表 / Generate random number list
(define (generate-random-list n)
  (let ((result '()))
    (dotimes (i n)
      (push (rand 1000) result))
    result))

;; 验证排序结果 / Verify sort result
(define (is-sorted? lst)
  (if (or (empty? lst) (= (length lst) 1))
      true
      (and (<= (first lst) (first (rest lst)))
           (is-sorted? (rest lst)))))

;; 基准测试 / Benchmark test
(define (benchmark-quicksort)
  (println "=== Quick Sort Benchmark ===")
  
  (set 'test-sizes '(100 500 1000 2000))
  
  (dolist (size test-sizes)
    (println "Testing with " size " elements:")
    
    ;; 生成测试数据 / Generate test data
    (set 'test-data (generate-random-list size))
    
    ;; 执行排序并计时 / Execute sort and measure time
    (set 'start-time (time-of-day))
    (set 'sorted-data (quicksort test-data))
    (set 'sort-time (- (time-of-day) start-time))
    
    ;; 验证结果 / Verify result
    (set 'is-correct (is-sorted? sorted-data))
    
    (println "  Time: " sort-time "ms")
    (println "  Correct: " is-correct)
    (println "  Elements per ms: " (div size sort-time))
    (println)))

;; 运行基准测试 / Run benchmark
(benchmark-quicksort)

;; 编译优化目标 / Compilation optimization targets:
;; 列表操作优化 / List operation optimization
;; 函数调用内联 / Function call inlining
;; 尾递归优化 / Tail recursion optimization
;; 期望整体3-8倍性能提升 / Expected overall 3-8x performance improvement
```

## 使用说明 / Usage Instructions

### 编译单个示例 / Compile Single Example

```bash
# 编译基础示例
./nlcc examples/basic/hello.lsp -o hello
./hello

# 编译高级示例
./nlcc examples/advanced/recursion.lsp -o recursion_test
./recursion_test

# 编译性能基准
./nlcc examples/benchmarks/fibonacci.lsp -o fib_benchmark
./fib_benchmark
```

### 批量测试 / Batch Testing

```bash
#!/bin/bash
# 批量编译和测试所有示例

echo "=== Compiling All Examples ==="

# 编译基础示例
for file in examples/basic/*.lsp; do
    echo "Compiling $file..."
    ./nlcc "$file" -o "${file%.lsp}"
done

# 编译高级示例
for file in examples/advanced/*.lsp; do
    echo "Compiling $file..."
    ./nlcc "$file" -o "${file%.lsp}"
done

# 编译基准测试
for file in examples/benchmarks/*.lsp; do
    echo "Compiling $file..."
    ./nlcc "$file" -o "${file%.lsp}"
done

echo "=== Running Performance Benchmarks ==="

# 运行基准测试
./examples/benchmarks/fibonacci
./examples/benchmarks/quicksort

echo "All examples compiled and tested successfully!"
```

### 性能对比测试 / Performance Comparison Test

```bash
#!/bin/bash
# 性能对比测试脚本

compare_performance() {
    local script=$1
    local name=$2
    
    echo "=== Performance Comparison: $name ==="
    
    # 解释执行
    echo "Interpreted execution:"
    time newlisp "$script"
    
    # 编译执行
    echo "Compiled execution:"
    ./nlcc "$script" -o "${script%.lsp}_compiled"
    time "./${script%.lsp}_compiled"
    
    echo ""
}

# 对比各个基准测试
compare_performance "examples/benchmarks/fibonacci.lsp" "Fibonacci"
compare_performance "examples/benchmarks/quicksort.lsp" "QuickSort"
compare_performance "examples/advanced/recursion.lsp" "Recursion"
```

这些示例代码展示了newLISP二进制编译器在不同场景下的应用和性能优化效果，为用户提供了完整的使用参考。
