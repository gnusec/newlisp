;; 简化的算术测试 / Simplified arithmetic test
;; 只包含我们编译器支持的基本功能
;; Only includes basic functionality supported by our compiler

(println ">>>>> Testing basic arithmetic")

;; 加法测试 / Addition tests
(+ 1 2)
(+ 10 20)
(+ 1 2 3)

;; 减法测试 / Subtraction tests
(- 10 5)
(- 100 50)
(- 20 5 3)

;; 乘法测试 / Multiplication tests
(* 2 3)
(* 5 6)
(* 2 3 4)

;; 除法测试 / Division tests
(/ 10 2)
(/ 100 4)
(/ 60 2 3)

;; 浮点数测试 / Float tests
(+ 1.5 2.5)
(* 3.14 2.0)

(println ">>>>> Basic arithmetic test SUCCESSFUL")
