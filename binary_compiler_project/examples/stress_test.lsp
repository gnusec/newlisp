;; 压力测试 / Stress test
;; 测试编译器处理大量表达式的能力
;; Test compiler's ability to handle many expressions

(println "Starting stress test...")

;; 大量算术运算 / Many arithmetic operations
(+ 1 1)
(+ 2 2)
(+ 3 3)
(+ 4 4)
(+ 5 5)
(+ 6 6)
(+ 7 7)
(+ 8 8)
(+ 9 9)
(+ 10 10)

(* 1 2)
(* 2 3)
(* 3 4)
(* 4 5)
(* 5 6)
(* 6 7)
(* 7 8)
(* 8 9)
(* 9 10)
(* 10 11)

;; 复杂嵌套 / Complex nesting
(+ (* 2 3) (* 4 5))
(+ (* 6 7) (* 8 9))
(+ (* 10 11) (* 12 13))
(+ (* 14 15) (* 16 17))
(+ (* 18 19) (* 20 21))

;; 多参数运算 / Multi-argument operations
(+ 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15)
(* 1 2 3 4 5)
(+ 100 200 300 400 500 600 700 800 900 1000)

;; 字符串输出 / String output
(println "Test 1")
(println "Test 2")
(println "Test 3")
(println "Test 4")
(println "Test 5")

;; 浮点数计算 / Floating point calculations
1.1
2.2
3.3
4.4
5.5
(+ 1.1 2.2 3.3)
(* 2.5 4.0)

(println "Stress test complete!")
(println ">>>>> Stress test SUCCESSFUL")
