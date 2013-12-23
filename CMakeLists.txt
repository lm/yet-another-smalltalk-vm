cmake_minimum_required(VERSION 2.8)
project(myvm)

#set(CMAKE_BUILD_TYPE "RelWithDebInfo")
#set(CMAKE_BUILD_TYPE "Release")
#set(CMAKE_C_FLAGS "-std=gnu99")
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_C_FLAGS "-std=gnu99 -pedantic -Wno-flexible-array-extensions -Wno-c11-extensions -Wno-gnu")
#set(CMAKE_C_FLAGS "-std=gnu99 -pedantic -Wno-flexible-array-extensions -Wno-c11-extensions -fsanitize=undefined")

#find_package(Threads)

add_library(CityHash STATIC cityhash/city.c)
add_library(Linenoise STATIC linenoise/linenoise.c)

set(sources
	vm/Bootstrap.c
	vm/Class.c
	vm/CodeGeneratorX64.c
	vm/Collection.c
	vm/CompiledCode.c
	vm/Compiler.c
	vm/Dictionary.c
	vm/Entry.c
	vm/Exception.c
	vm/GarbageCollector.c
	vm/Handle.c
	vm/Heap.c
	vm/HeapPage.c
	vm/Iterator.c
	vm/Lookup.c
	vm/Optimizer.c
	vm/OsLinux.c
	vm/Parser.c
	vm/Primitives.c
	vm/RegisterAllocator.c
	vm/Repl.c
	vm/Scavenger.c
	vm/Scope.c
	vm/Smalltalk.c
	vm/Snapshot.c
	vm/Socket.c
	vm/StackFrame.c
	vm/Stream.c
	vm/String.c
	vm/StubCodeX64.c
	vm/Thread.c
	vm/Tokenizer.c
)

add_library(VM STATIC ${sources})
target_link_libraries(VM CityHash)

add_executable(st main.c)
target_link_libraries(st ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(st VM)
target_link_libraries(st Linenoise)
