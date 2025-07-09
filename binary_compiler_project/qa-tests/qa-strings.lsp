;; 字符串测试 / String tests
;; 测试编译器的字符串处理功能
;; Test compiler's string handling functionality

(println ">>>>> Testing string operations")

;; 简单字符串 / Simple strings
"Hello"
"World"
"newLISP"

;; 带空格的字符串 / Strings with spaces
"Hello World"
"This is a test"
"String with numbers 123"

;; 特殊字符 / Special characters
"String with symbols !@#$%"
"Path/to/file.txt"
"Email@domain.com"

;; 长字符串 / Long strings
"This is a very long string that tests the compiler's ability to handle longer text content"

;; 空字符串 / Empty string
""

;; 字符串输出测试 / String output tests
(println "Testing println with strings")
(println "Line 1")
(println "Line 2")
(println "Line 3")

;; 混合内容 / Mixed content
(println "Number: ")
42
(println "Float: ")
3.14159
(println "String: ")
"Test complete"

(println ">>>>> String operations SUCCESSFUL")
