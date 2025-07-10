;; 简单数学测试 / Simple math test
;; 基于qa-factorfibo的简化版本，只测试基本数学运算
;; Simplified version based on qa-factorfibo, testing only basic math operations

(println ">>>>> Testing simple math operations")

;; 基本算术 / Basic arithmetic
(+ 1 1)
(+ 2 3)
(+ 5 8)
(+ 13 21)

;; 乘法运算 / Multiplication
(* 2 2)
(* 3 3)
(* 5 5)
(* 7 7)

;; 除法运算 / Division
(/ 10 2)
(/ 21 3)
(/ 64 8)
(/ 100 4)

;; 减法运算 / Subtraction
(- 10 5)
(- 21 8)
(- 100 37)
(- 1000 999)

;; 复合运算 / Compound operations
(+ (* 2 3) (* 4 5))
(- (* 10 10) (* 5 5))
(/ (* 6 8) (* 2 3))
(+ (/ 20 4) (* 3 7))

;; 大数运算 / Large number operations
(+ 1000000 2000000)
(* 1000 1000)
(/ 1000000 1000)
(- 9999999 1000000)

;; 多参数运算 / Multi-argument operations
(+ 1 2 3 4 5 6 7 8 9 10)
(* 1 2 3 4 5)
(+ 100 200 300 400 500)

;; 浮点数学 / Floating point math
(+ 3.14159 2.71828)
(* 2.5 4.0)
(/ 22.0 7.0)
(- 10.5 3.7)

;; 嵌套复杂运算 / Nested complex operations
(+ (* (+ 1 2) (+ 3 4)) (* (+ 5 6) (+ 7 8)))
(- (* (+ 10 20) (+ 30 40)) (* (+ 5 15) (+ 25 35)))

(println ">>>>> Simple math operations SUCCESSFUL")
