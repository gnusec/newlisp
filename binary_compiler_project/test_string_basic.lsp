; 基础字符串操作测试
; Basic string operations test

; 测试字符串长度
(define test_string "Hello World")
(println "Test string:")
(println test_string)

(define str_len (string-length test_string))
(println "String length:")
(println str_len)

; 测试空字符串
(define empty_str "")
(define empty_len (string-length empty_str))
(println "Empty string length:")
(println empty_len)

(println "String operations test completed")
