# Alternative GNU Make workspace makefile autogenerated by Premake

ifndef config
  config=debug_x64
endif

ifndef verbose
  SILENT = @
endif

ifeq ($(config),debug_x64)
  x_stb_config = debug_x64
  x_glad_config = debug_x64
  x_glfw_config = debug_x64
  x_catch2_config = debug_x64
  x_rapidobj_config = debug_x64
  x_fontstash_config = debug_x64
  main_config = debug_x64
  main_shaders_config = debug_x64
  vmlib_test_config = debug_x64
  support_config = debug_x64
  vmlib_config = debug_x64

else ifeq ($(config),release_x64)
  x_stb_config = release_x64
  x_glad_config = release_x64
  x_glfw_config = release_x64
  x_catch2_config = release_x64
  x_rapidobj_config = release_x64
  x_fontstash_config = release_x64
  main_config = release_x64
  main_shaders_config = release_x64
  vmlib_test_config = release_x64
  support_config = release_x64
  vmlib_config = release_x64

else
  $(error "invalid configuration $(config)")
endif

PROJECTS := x-stb x-glad x-glfw x-catch2 x-rapidobj x-fontstash main main-shaders vmlib-test support vmlib

.PHONY: all clean help $(PROJECTS) 

all: $(PROJECTS)

x-stb:
ifneq (,$(x_stb_config))
	@echo "==== Building x-stb ($(x_stb_config)) ===="
	@${MAKE} --no-print-directory -C third_party -f x-stb.make config=$(x_stb_config)
endif

x-glad:
ifneq (,$(x_glad_config))
	@echo "==== Building x-glad ($(x_glad_config)) ===="
	@${MAKE} --no-print-directory -C third_party -f x-glad.make config=$(x_glad_config)
endif

x-glfw:
ifneq (,$(x_glfw_config))
	@echo "==== Building x-glfw ($(x_glfw_config)) ===="
	@${MAKE} --no-print-directory -C third_party -f x-glfw.make config=$(x_glfw_config)
endif

x-catch2:
ifneq (,$(x_catch2_config))
	@echo "==== Building x-catch2 ($(x_catch2_config)) ===="
	@${MAKE} --no-print-directory -C third_party -f x-catch2.make config=$(x_catch2_config)
endif

x-rapidobj:
ifneq (,$(x_rapidobj_config))
	@echo "==== Building x-rapidobj ($(x_rapidobj_config)) ===="
	@${MAKE} --no-print-directory -C third_party -f x-rapidobj.make config=$(x_rapidobj_config)
endif

x-fontstash:
ifneq (,$(x_fontstash_config))
	@echo "==== Building x-fontstash ($(x_fontstash_config)) ===="
	@${MAKE} --no-print-directory -C third_party -f x-fontstash.make config=$(x_fontstash_config)
endif

main: vmlib support x-stb x-glad x-glfw x-fontstash main-shaders x-rapidobj
ifneq (,$(main_config))
	@echo "==== Building main ($(main_config)) ===="
	@${MAKE} --no-print-directory -C main -f Makefile config=$(main_config)
endif

main-shaders:
ifneq (,$(main_shaders_config))
	@echo "==== Building main-shaders ($(main_shaders_config)) ===="
	@${MAKE} --no-print-directory -C assets/cw2 -f Makefile config=$(main_shaders_config)
endif

vmlib-test: vmlib x-catch2
ifneq (,$(vmlib_test_config))
	@echo "==== Building vmlib-test ($(vmlib_test_config)) ===="
	@${MAKE} --no-print-directory -C vmlib-test -f Makefile config=$(vmlib_test_config)
endif

support:
ifneq (,$(support_config))
	@echo "==== Building support ($(support_config)) ===="
	@${MAKE} --no-print-directory -C support -f Makefile config=$(support_config)
endif

vmlib:
ifneq (,$(vmlib_config))
	@echo "==== Building vmlib ($(vmlib_config)) ===="
	@${MAKE} --no-print-directory -C vmlib -f Makefile config=$(vmlib_config)
endif

clean:
	@${MAKE} --no-print-directory -C third_party -f x-stb.make clean
	@${MAKE} --no-print-directory -C third_party -f x-glad.make clean
	@${MAKE} --no-print-directory -C third_party -f x-glfw.make clean
	@${MAKE} --no-print-directory -C third_party -f x-catch2.make clean
	@${MAKE} --no-print-directory -C third_party -f x-rapidobj.make clean
	@${MAKE} --no-print-directory -C third_party -f x-fontstash.make clean
	@${MAKE} --no-print-directory -C main -f Makefile clean
	@${MAKE} --no-print-directory -C assets/cw2 -f Makefile clean
	@${MAKE} --no-print-directory -C vmlib-test -f Makefile clean
	@${MAKE} --no-print-directory -C support -f Makefile clean
	@${MAKE} --no-print-directory -C vmlib -f Makefile clean

help:
	@echo "Usage: make [config=name] [target]"
	@echo ""
	@echo "CONFIGURATIONS:"
	@echo "  debug_x64"
	@echo "  release_x64"
	@echo ""
	@echo "TARGETS:"
	@echo "   all (default)"
	@echo "   clean"
	@echo "   x-stb"
	@echo "   x-glad"
	@echo "   x-glfw"
	@echo "   x-catch2"
	@echo "   x-rapidobj"
	@echo "   x-fontstash"
	@echo "   main"
	@echo "   main-shaders"
	@echo "   vmlib-test"
	@echo "   support"
	@echo "   vmlib"
	@echo ""
	@echo "For more information, see https://github.com/premake/premake-core/wiki"