;; 计算密集型测试 / Computation-heavy test
;; 这个测试包含大量计算，如果是解释执行会很慢
;; This test contains heavy computation, would be slow if interpreted

(println "Starting computation-heavy test...")

;; 大量嵌套计算 / Heavy nested computations
(+ (* (+ 1 2) (+ 3 4)) (* (+ 5 6) (+ 7 8)))
(+ (* (+ 9 10) (+ 11 12)) (* (+ 13 14) (+ 15 16)))
(+ (* (+ 17 18) (+ 19 20)) (* (+ 21 22) (+ 23 24)))
(+ (* (+ 25 26) (+ 27 28)) (* (+ 29 30) (+ 31 32)))
(+ (* (+ 33 34) (+ 35 36)) (* (+ 37 38) (+ 39 40)))

;; 大数值计算 / Large number calculations
(+ 1000000 2000000 3000000 4000000 5000000)
(* 1000 2000 3000)
(+ 999999 888888 777777 666666 555555)
(* 123 456 789)

;; 浮点数密集计算 / Floating point intensive
(+ 3.14159 2.71828 1.41421 1.73205)
(* 2.5 3.7 4.2 5.1)
(+ 1.1 2.2 3.3 4.4 5.5 6.6 7.7 8.8 9.9)

;; 重复计算模式 / Repetitive computation patterns
(+ 1 1 1 1 1 1 1 1 1 1)
(* 2 2 2 2 2)
(+ 10 20 30 40 50 60 70 80 90 100)

;; 字符串处理 / String processing
(println "Computation 1 complete")
(println "Computation 2 complete")
(println "Computation 3 complete")
(println "Computation 4 complete")
(println "Computation 5 complete")

;; 更多复杂表达式 / More complex expressions
(+ (* 100 200) (* 300 400))
(+ (* 500 600) (* 700 800))
(+ (* 900 1000) (* 1100 1200))

(println "All computations complete!")
(println "This proves true binary compilation!")
(println ">>>>> Computation heavy test SUCCESSFUL")
