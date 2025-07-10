;; 性能基准测试 / Performance benchmark test
;; 测试编译器生成的二进制代码的执行性能
;; Test execution performance of compiler-generated binary code

(println ">>>>> Testing performance benchmark")

;; 大量算术运算 / Heavy arithmetic operations
(+ 1 2 3 4 5 6 7 8 9 10)
(+ 11 12 13 14 15 16 17 18 19 20)
(+ 21 22 23 24 25 26 27 28 29 30)
(+ 31 32 33 34 35 36 37 38 39 40)
(+ 41 42 43 44 45 46 47 48 49 50)

(* 1 2 3 4 5)
(* 6 7 8 9 10)
(* 11 12 13 14 15)
(* 16 17 18 19 20)
(* 21 22 23 24 25)

;; 复杂嵌套运算 / Complex nested operations
(+ (* (+ 1 2) (+ 3 4)) (* (+ 5 6) (+ 7 8)))
(+ (* (+ 9 10) (+ 11 12)) (* (+ 13 14) (+ 15 16)))
(+ (* (+ 17 18) (+ 19 20)) (* (+ 21 22) (+ 23 24)))
(+ (* (+ 25 26) (+ 27 28)) (* (+ 29 30) (+ 31 32)))
(+ (* (+ 33 34) (+ 35 36)) (* (+ 37 38) (+ 39 40)))

;; 浮点数密集计算 / Floating point intensive computation
(+ 1.1 2.2 3.3 4.4 5.5)
(+ 6.6 7.7 8.8 9.9 10.10)
(* 1.5 2.5 3.5 4.5)
(* 5.5 6.5 7.5 8.5)

;; 大数值运算 / Large number operations
(+ 1000000 2000000 3000000)
(+ 4000000 5000000 6000000)
(* 1000 2000 3000)
(* 4000 5000 6000)

;; 重复模式计算 / Repetitive pattern computation
(+ 100 100 100 100 100)
(+ 200 200 200 200 200)
(+ 300 300 300 300 300)
(+ 400 400 400 400 400)
(+ 500 500 500 500 500)

(* 10 10 10 10)
(* 20 20 20 20)
(* 30 30 30 30)
(* 40 40 40 40)
(* 50 50 50 50)

;; 深度嵌套测试 / Deep nesting test
(+ (+ (+ (+ 1 2) (+ 3 4)) (+ (+ 5 6) (+ 7 8))) (+ (+ (+ 9 10) (+ 11 12)) (+ (+ 13 14) (+ 15 16))))
(* (* (* (* 2 2) (* 3 3)) (* (* 4 4) (* 5 5))) (* (* (* 6 6) (* 7 7)) (* (* 8 8) (* 9 9))))

;; 混合运算压力测试 / Mixed operation stress test
(+ (* 123 456) (- 789 123))
(+ (* 234 567) (- 890 234))
(+ (* 345 678) (- 901 345))
(+ (* 456 789) (- 012 456))
(+ (* 567 890) (- 123 567))

;; 字符串输出测试 / String output test
(println "Performance test section 1 complete")
(println "Performance test section 2 complete")
(println "Performance test section 3 complete")
(println "Performance test section 4 complete")
(println "Performance test section 5 complete")

;; 最终大型计算 / Final large computation
(+ (* (+ 1000 2000) (+ 3000 4000)) (* (+ 5000 6000) (+ 7000 8000)))

(println ">>>>> Performance benchmark SUCCESSFUL")
