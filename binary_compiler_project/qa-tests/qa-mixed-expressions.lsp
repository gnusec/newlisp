;; 混合表达式测试 / Mixed expressions test
;; 测试编译器处理各种混合表达式的能力
;; Test compiler's ability to handle various mixed expressions

(println ">>>>> Testing mixed expressions")

;; 数字和字符串混合 / Numbers and strings mixed
(println "Testing number output:")
123
456
789

(println "Testing float output:")
1.23
4.56
7.89

(println "Testing string output:")
"First string"
"Second string"
"Third string"

;; 算术运算和输出混合 / Arithmetic and output mixed
(println "Arithmetic results:")
(+ 10 20)
(println "Addition complete")

(* 5 6)
(println "Multiplication complete")

(- 100 25)
(println "Subtraction complete")

(/ 50 2)
(println "Division complete")

;; 复杂嵌套表达式 / Complex nested expressions
(println "Complex expressions:")
(+ (* 2 3) (* 4 5))
(+ (* (+ 1 2) 3) (* (+ 4 5) 2))
(* (+ 10 20) (- 50 30))

;; 多种数据类型 / Multiple data types
(println "Multiple data types:")
42
"Forty-two"
3.14159
"Pi approximation"
(+ 20 22)
"Sum of 20 and 22"

;; 大量表达式 / Many expressions
1
2
3
4
5
"One"
"Two"
"Three"
"Four"
"Five"

(println ">>>>> Mixed expressions SUCCESSFUL")
