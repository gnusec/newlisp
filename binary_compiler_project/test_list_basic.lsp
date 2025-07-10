; 基础列表操作测试
; Basic list operations test

; 测试创建单元素列表
(define my_list (list 42))
(println "Created list with value 42")

; 测试获取第一个元素
(define first_value (first my_list))
(println "First element:")
(println first_value)

; 测试获取列表长度
(define list_length (length my_list))
(println "List length:")
(println list_length)

(println "List operations test completed")
