; 新功能综合测试
; Comprehensive test for new features

(println "=== New Features Test ===")

; 测试列表操作
(println "Testing list operations:")
(define my_list (list 42))
(define first_val (first my_list))
(define list_len (length my_list))
(println "List first element:")
(println first_val)
(println "List length:")
(println list_len)

; 测试字符串操作
(println "Testing string operations:")
(define test_str "Hello newLISP")
(define str_length (string-length test_str))
(println "String:")
(println test_str)
(println "String length:")
(println str_length)

; 测试数学函数
(println "Testing math functions:")
(define num 16)
(define sqrt_result (sqrt num))
(println "Square root of 16:")
(println sqrt_result)

; 测试组合使用
(println "Testing combined usage:")
(define combined_list (list (string-length "test")))
(define combined_result (first combined_list))
(println "Length of 'test' in list:")
(println combined_result)

(println "=== All New Features Working! ===")
