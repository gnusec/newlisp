# newLISP Binary Compiler Makefile
# 构建系统配置 / Build System Configuration

# 编译器设置 / Compiler Settings
CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -O2 -g
CFLAGS_DEBUG = -std=c99 -Wall -Wextra -Wpedantic -O0 -g -DDEBUG -DMEMORY_DEBUG
CFLAGS_RELEASE = -std=c99 -Wall -Wextra -Wpedantic -O3 -DNDEBUG

# 目录设置 / Directory Settings
SRC_DIR = src
BUILD_DIR = build
TEST_DIR = tests
RUNTIME_DIR = $(SRC_DIR)/runtime
COMPILER_DIR = $(SRC_DIR)/compiler
AST_DIR = $(SRC_DIR)/ast_converter
TINYCC_DIR = $(SRC_DIR)/tinycc_integration

# 包含路径 / Include Paths
INCLUDES = -I$(SRC_DIR) -I$(RUNTIME_DIR) -I$(COMPILER_DIR) -I$(AST_DIR) -I$(TINYCC_DIR)

# 库设置 / Library Settings
LIBS = -lm -lpthread
TINYCC_LIBS = -ltcc

# 源文件 / Source Files
RUNTIME_SOURCES = $(wildcard $(RUNTIME_DIR)/*.c)
COMPILER_SOURCES = $(wildcard $(COMPILER_DIR)/*.c)
AST_SOURCES = $(wildcard $(AST_DIR)/*.c)
TINYCC_SOURCES = $(wildcard $(TINYCC_DIR)/*.c)

# 对象文件 / Object Files
RUNTIME_OBJECTS = $(RUNTIME_SOURCES:$(RUNTIME_DIR)/%.c=$(BUILD_DIR)/runtime/%.o)
COMPILER_OBJECTS = $(COMPILER_SOURCES:$(COMPILER_DIR)/%.c=$(BUILD_DIR)/compiler/%.o)
AST_OBJECTS = $(AST_SOURCES:$(AST_DIR)/%.c=$(BUILD_DIR)/ast/%.o)
TINYCC_OBJECTS = $(TINYCC_SOURCES:$(TINYCC_DIR)/%.c=$(BUILD_DIR)/tinycc/%.o)

ALL_OBJECTS = $(RUNTIME_OBJECTS) $(COMPILER_OBJECTS) $(AST_OBJECTS) $(TINYCC_OBJECTS)

# 测试文件 / Test Files
TEST_SOURCES = $(wildcard $(TEST_DIR)/unit/*.c)
TEST_OBJECTS = $(TEST_SOURCES:$(TEST_DIR)/unit/%.c=$(BUILD_DIR)/tests/%.o)
TEST_EXECUTABLES = $(TEST_SOURCES:$(TEST_DIR)/unit/%.c=$(BUILD_DIR)/tests/%)

# 目标文件 / Target Files
NLCC_EXECUTABLE = $(BUILD_DIR)/nlcc
RUNTIME_LIBRARY = $(BUILD_DIR)/libnlruntime.a
TEST_RUNNER = $(BUILD_DIR)/test_runner

# 默认目标 / Default Target
.PHONY: all
all: directories $(RUNTIME_LIBRARY) $(NLCC_EXECUTABLE)

# MVP快速构建 / MVP Quick Build
.PHONY: mvp
mvp: directories $(RUNTIME_LIBRARY) $(AST_OBJECTS) $(TINYCC_OBJECTS)
	@echo "MVP core components built successfully"

# 创建目录 / Create Directories
.PHONY: directories
directories:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR)/runtime
	@mkdir -p $(BUILD_DIR)/compiler
	@mkdir -p $(BUILD_DIR)/ast
	@mkdir -p $(BUILD_DIR)/tinycc
	@mkdir -p $(BUILD_DIR)/tests

# 运行时库 / Runtime Library
$(RUNTIME_LIBRARY): $(RUNTIME_OBJECTS)
	@echo "Creating runtime library..."
	ar rcs $@ $^
	@echo "Runtime library created: $@"

# 编译器可执行文件 / Compiler Executable
$(NLCC_EXECUTABLE): $(COMPILER_OBJECTS) $(AST_OBJECTS) $(TINYCC_OBJECTS) $(RUNTIME_LIBRARY)
	@echo "Linking compiler executable..."
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
	@echo "Compiler executable created: $@"

# 运行时对象文件 / Runtime Object Files
$(BUILD_DIR)/runtime/%.o: $(RUNTIME_DIR)/%.c
	@echo "Compiling runtime: $<"
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# 编译器对象文件 / Compiler Object Files
$(BUILD_DIR)/compiler/%.o: $(COMPILER_DIR)/%.c
	@echo "Compiling compiler: $<"
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# AST对象文件 / AST Object Files
$(BUILD_DIR)/ast/%.o: $(AST_DIR)/%.c
	@echo "Compiling AST: $<"
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# TinyCC对象文件 / TinyCC Object Files
$(BUILD_DIR)/tinycc/%.o: $(TINYCC_DIR)/%.c
	@echo "Compiling TinyCC integration: $<"
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# 测试目标 / Test Targets
.PHONY: test
test: test-infrastructure test-mvp test-parser-codegen test-unit test-integration

# 基础设施测试 / Infrastructure Tests
.PHONY: test-infrastructure
test-infrastructure: $(BUILD_DIR)/tests/test_infrastructure
	@echo "Running infrastructure tests..."
	$<

$(BUILD_DIR)/tests/test_infrastructure: $(TEST_DIR)/unit/test_infrastructure.c $(RUNTIME_LIBRARY)
	@echo "Building infrastructure tests..."
	$(CC) $(CFLAGS_DEBUG) $(INCLUDES) -o $@ $< $(RUNTIME_LIBRARY) $(LIBS)

# MVP核心功能测试 / MVP Core Functionality Tests
.PHONY: test-mvp
test-mvp: $(BUILD_DIR)/tests/test_mvp_core
	@echo "Running MVP core tests..."
	$<

$(BUILD_DIR)/tests/test_mvp_core: $(TEST_DIR)/unit/test_mvp_core.c $(RUNTIME_LIBRARY) $(AST_OBJECTS) $(TINYCC_OBJECTS)
	@echo "Building MVP core tests..."
	$(CC) $(CFLAGS_DEBUG) $(INCLUDES) -o $@ $< $(RUNTIME_LIBRARY) $(AST_OBJECTS) $(TINYCC_OBJECTS) $(LIBS)

# 解析器和代码生成器集成测试 / Parser and Code Generator Integration Tests
.PHONY: test-parser-codegen
test-parser-codegen: $(BUILD_DIR)/tests/test_parser_codegen
	@echo "Running parser and code generator integration tests..."
	$<

$(BUILD_DIR)/tests/test_parser_codegen: $(TEST_DIR)/unit/test_parser_codegen.c $(RUNTIME_LIBRARY) $(AST_OBJECTS) $(TINYCC_OBJECTS)
	@echo "Building parser and code generator integration tests..."
	$(CC) $(CFLAGS_DEBUG) $(INCLUDES) -o $@ $< $(RUNTIME_LIBRARY) $(AST_OBJECTS) $(TINYCC_OBJECTS) $(LIBS)

# 单元测试 / Unit Tests
.PHONY: test-unit
test-unit: directories $(RUNTIME_LIBRARY)
	@echo "Running unit tests..."
	@for test_file in $(TEST_DIR)/unit/test_*.c; do \
		if [ -f "$$test_file" ]; then \
			test_name=$$(basename "$$test_file" .c); \
			echo "Building and running $$test_name..."; \
			$(CC) $(CFLAGS_DEBUG) $(INCLUDES) -o $(BUILD_DIR)/tests/$$test_name $$test_file $(RUNTIME_LIBRARY) $(LIBS); \
			$(BUILD_DIR)/tests/$$test_name; \
		fi \
	done

# 集成测试 / Integration Tests
.PHONY: test-integration
test-integration: $(BUILD_DIR)/tests/test_end_to_end
	@echo "Running integration tests..."
	$<

$(BUILD_DIR)/tests/test_end_to_end: $(TEST_DIR)/integration/test_end_to_end.c $(RUNTIME_LIBRARY) $(AST_OBJECTS) $(TINYCC_OBJECTS)
	@echo "Building end-to-end integration tests..."
	$(CC) $(CFLAGS_DEBUG) $(INCLUDES) -o $@ $< $(RUNTIME_LIBRARY) $(AST_OBJECTS) $(TINYCC_OBJECTS) $(LIBS)

# 性能测试 / Performance Tests
.PHONY: test-performance
test-performance: $(NLCC_EXECUTABLE)
	@echo "Running performance tests..."
	@if [ -d "$(TEST_DIR)/performance" ]; then \
		for test_script in $(TEST_DIR)/performance/*.sh; do \
			if [ -f "$$test_script" ]; then \
				echo "Running performance test: $$test_script"; \
				bash "$$test_script"; \
			fi \
		done \
	fi

# 内存检查 / Memory Check
.PHONY: memcheck
memcheck: $(BUILD_DIR)/tests/test_infrastructure
	@echo "Running memory check..."
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes $<

# 代码覆盖率 / Code Coverage
.PHONY: coverage
coverage: CFLAGS += --coverage
coverage: clean test
	@echo "Generating coverage report..."
	gcov $(RUNTIME_SOURCES) $(COMPILER_SOURCES)
	lcov --capture --directory . --output-file coverage.info
	genhtml coverage.info --output-directory coverage_html

# 静态分析 / Static Analysis
.PHONY: analyze
analyze:
	@echo "Running static analysis..."
	cppcheck --enable=all --std=c99 $(SRC_DIR)

# 格式化代码 / Format Code
.PHONY: format
format:
	@echo "Formatting code..."
	find $(SRC_DIR) -name "*.c" -o -name "*.h" | xargs clang-format -i
	find $(TEST_DIR) -name "*.c" -o -name "*.h" | xargs clang-format -i

# 调试构建 / Debug Build
.PHONY: debug
debug: CFLAGS = $(CFLAGS_DEBUG)
debug: clean all

# 发布构建 / Release Build
.PHONY: release
release: CFLAGS = $(CFLAGS_RELEASE)
release: clean all

# 安装 / Install
.PHONY: install
install: $(NLCC_EXECUTABLE)
	@echo "Installing newLISP compiler..."
	install -d $(DESTDIR)/usr/local/bin
	install -m 755 $(NLCC_EXECUTABLE) $(DESTDIR)/usr/local/bin/nlcc
	@echo "Installation complete"

# 卸载 / Uninstall
.PHONY: uninstall
uninstall:
	@echo "Uninstalling newLISP compiler..."
	rm -f $(DESTDIR)/usr/local/bin/nlcc
	@echo "Uninstallation complete"

# 清理 / Clean
.PHONY: clean
clean:
	@echo "Cleaning build files..."
	rm -rf $(BUILD_DIR)
	rm -f *.gcov *.gcda *.gcno coverage.info
	rm -rf coverage_html

# 深度清理 / Deep Clean
.PHONY: distclean
distclean: clean
	@echo "Deep cleaning..."
	find . -name "*~" -delete
	find . -name "*.bak" -delete
	find . -name "core" -delete

# 帮助信息 / Help Information
.PHONY: help
help:
	@echo "newLISP Binary Compiler Build System"
	@echo "===================================="
	@echo ""
	@echo "Available targets:"
	@echo "  all              - Build everything (default)"
	@echo "  debug            - Build with debug flags"
	@echo "  release          - Build with release flags"
	@echo "  test             - Run all tests"
	@echo "  test-infrastructure - Run infrastructure tests"
	@echo "  test-unit        - Run unit tests"
	@echo "  test-integration - Run integration tests"
	@echo "  test-performance - Run performance tests"
	@echo "  memcheck         - Run memory check with valgrind"
	@echo "  coverage         - Generate code coverage report"
	@echo "  analyze          - Run static analysis"
	@echo "  format           - Format source code"
	@echo "  install          - Install the compiler"
	@echo "  uninstall        - Uninstall the compiler"
	@echo "  clean            - Clean build files"
	@echo "  distclean        - Deep clean all generated files"
	@echo "  help             - Show this help message"
	@echo ""
	@echo "Environment variables:"
	@echo "  CC               - C compiler (default: gcc)"
	@echo "  CFLAGS           - Additional compiler flags"
	@echo "  DESTDIR          - Installation prefix"

# 依赖关系 / Dependencies
-include $(ALL_OBJECTS:.o=.d)

# 自动生成依赖关系 / Auto-generate Dependencies
$(BUILD_DIR)/%.d: %.c
	@mkdir -p $(dir $@)
	$(CC) $(INCLUDES) -MM -MT $(@:.d=.o) $< > $@

# 显示构建信息 / Show Build Information
.PHONY: info
info:
	@echo "Build Information:"
	@echo "=================="
	@echo "CC: $(CC)"
	@echo "CFLAGS: $(CFLAGS)"
	@echo "INCLUDES: $(INCLUDES)"
	@echo "LIBS: $(LIBS)"
	@echo "BUILD_DIR: $(BUILD_DIR)"
	@echo "SRC_DIR: $(SRC_DIR)"
	@echo ""
	@echo "Source files:"
	@echo "Runtime: $(words $(RUNTIME_SOURCES)) files"
	@echo "Compiler: $(words $(COMPILER_SOURCES)) files"
	@echo "AST: $(words $(AST_SOURCES)) files"
	@echo "TinyCC: $(words $(TINYCC_SOURCES)) files"
	@echo "Tests: $(words $(TEST_SOURCES)) files"
