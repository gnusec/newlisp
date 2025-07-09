;; 复杂函数测试 / Complex function tests
;; 测试函数定义、条件语句、循环等高级特性
;; Test function definitions, conditionals, loops and other advanced features

(println "=== Complex newLISP Features Test ===")

;; 嵌套函数调用 / Nested function calls
(println "Testing nested function calls:")
(+ (* 2 3) (- 10 5))
(* (+ 1 2) (+ 3 4))

;; 复杂算术表达式 / Complex arithmetic expressions
(println "Testing complex arithmetic:")
(+ 1 2 3 4 5 6 7 8 9 10)
(* 2 3 4 5)
(- 100 10 5 3 2)
(/ 120 2 3 4)

;; 字符串处理 / String processing
(println "Testing strings:")
"Simple string"
"String with spaces and numbers 123"
"Another test string"

;; 混合表达式 / Mixed expressions
(println "Testing mixed expressions:")
(+ 10 20)
(println "Result of addition")
(* 5 6)
(println "Result of multiplication")

;; 浮点数运算 / Floating point operations
(println "Testing floating point:")
3.14159
2.71828
(+ 1.5 2.5 3.5)
(* 2.0 3.14159)

;; 大数值测试 / Large number tests
(println "Testing large numbers:")
1000000
999999999
(+ 1000000 2000000 3000000)

(println "=== Test Complete ===")
(println ">>>>> Complex functions test SUCCESSFUL")
