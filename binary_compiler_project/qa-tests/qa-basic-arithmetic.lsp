;; 基础算术运算测试 / Basic arithmetic operations test
;; 测试编译器的基本算术功能
;; Test compiler's basic arithmetic functionality

(println ">>>>> Testing basic arithmetic operations")

;; 简单加法 / Simple addition
(+ 1 2)
(+ 10 20)
(+ 100 200 300)

;; 简单减法 / Simple subtraction  
(- 10 5)
(- 100 50)
(- 1000 200 300)

;; 简单乘法 / Simple multiplication
(* 2 3)
(* 5 6)
(* 2 3 4)

;; 简单除法 / Simple division
(/ 10 2)
(/ 100 4)
(/ 120 2 3)

;; 嵌套运算 / Nested operations
(+ (* 2 3) (* 4 5))
(- (* 10 5) (* 2 3))
(* (+ 1 2) (+ 3 4))
(/ (+ 10 20) (+ 2 3))

;; 多参数运算 / Multi-argument operations
(+ 1 2 3 4 5)
(* 1 2 3 4)
(+ 10 20 30 40 50)

;; 浮点数运算 / Floating point operations
3.14
2.71
(+ 1.5 2.5)
(* 3.14 2.0)

;; 大数值 / Large numbers
1000000
(+ 1000000 2000000)
(* 1000 2000)

(println ">>>>> Basic arithmetic operations SUCCESSFUL")
