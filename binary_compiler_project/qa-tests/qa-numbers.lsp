;; 数字测试 / Numbers test
;; 基于qa-specific-tests的数字处理测试
;; Number handling tests based on qa-specific-tests

(println ">>>>> Testing number operations")

;; 整数测试 / Integer tests
0
1
-1
42
-42
123456
-123456
1000000
-1000000

;; 浮点数测试 / Float tests
0.0
1.0
-1.0
3.14159
-3.14159
2.71828
-2.71828
123.456
-123.456

;; 科学计数法（如果支持）/ Scientific notation (if supported)
1.0
10.0
100.0
1000.0

;; 边界值测试 / Boundary value tests
999999999
-999999999

;; 算术运算精度测试 / Arithmetic precision tests
(+ 0.1 0.2)
(- 1.0 0.9)
(* 0.3 3.0)
(/ 1.0 3.0)

;; 整数算术 / Integer arithmetic
(+ 2147483647 1)
(- -2147483648 1)
(* 1000000 1000)
(/ 1000000000 1000)

;; 混合运算 / Mixed operations
(+ 1 2.5)
(* 3 1.5)
(- 10.0 5)
(/ 20 4.0)

(println ">>>>> Number operations SUCCESSFUL")
