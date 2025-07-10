;; 表达式测试 / Expression tests
;; 测试各种复杂表达式的编译和执行
;; Test compilation and execution of various complex expressions

(println ">>>>> Testing complex expressions")

;; 深度嵌套 / Deep nesting
(+ (+ (+ 1 2) (+ 3 4)) (+ (+ 5 6) (+ 7 8)))
(* (* (* 2 2) (* 3 3)) (* (* 4 4) (* 5 5)))

;; 多层运算 / Multi-level operations
(+ (* (+ 1 2) (- 10 5)) (* (+ 3 4) (- 20 15)))
(- (* (+ 10 20) (+ 30 40)) (* (+ 5 15) (+ 25 35)))

;; 长链运算 / Long chain operations
(+ 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20)
(* 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2)

;; 混合数据类型 / Mixed data types
(+ 1 2.0 3 4.5 5)
(* 2.5 4 1.5 8)

;; 复杂嵌套字符串和数字 / Complex nested strings and numbers
(println "Result 1:")
(+ 100 200)
(println "Result 2:")
(* 50 60)
(println "Result 3:")
(- 1000 500)
(println "Result 4:")
(/ 2000 40)

;; 极深嵌套 / Very deep nesting
(+ (+ (+ (+ 1 1) (+ 2 2)) (+ (+ 3 3) (+ 4 4))) (+ (+ (+ 5 5) (+ 6 6)) (+ (+ 7 7) (+ 8 8))))

;; 大量参数 / Many arguments
(+ 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1)

;; 浮点精度测试 / Float precision tests
(+ 0.1 0.1 0.1 0.1 0.1 0.1 0.1 0.1 0.1 0.1)
(* 1.1 1.1 1.1 1.1 1.1)

;; 字符串和计算混合 / Strings and calculations mixed
"Start"
(+ 10 20 30)
"Middle"
(* 5 6 7)
"End"

(println ">>>>> Complex expressions SUCCESSFUL")
