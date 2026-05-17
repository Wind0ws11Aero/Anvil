Anvil
A robust, low-overhead library that brings Object-Oriented Programming (OOP) and RAII (Resource Acquisition Is Initialization) capabilities to C, powered by Clang's Blocks extension.
Anvil provides the heavy-duty anvil upon which you can forge modern, safe, and structured C architectures without leaving the metal.
🚀 Features
•	True Encapsulation: Define classes with fields and bindable methods.
•	Lambda-like Methods: Methods are backed by Clang Blocks, allowing closures, context capturing, and dynamic re-binding at runtime.
•	Automatic Memory Management (RAII): Smart pointers (sptr) with explicit reference counting that clean themselves up automatically when going out of scope.
•	Zero Syntactic Noise: Intuitively use keywords like class, new, and delete directly in C.
🛠️ Requirements & Installation
Anvil relies heavily on the Clang Blocks extension (<Block.h>).
Compiler Support
You must compile your project using Clang with the -fblocks flag enabled, and link against the Blocks runtime (-lBlocksRuntime on Linux, enabled by default on macOS).
# For Linux
clang -fblocks raii.c your_program.c -lBlocksRuntime -o your_program

# For macOS
clang -fblocks raii.c your_program.c -o your_program

Usage Guide
1. Object-Oriented Programming (OOP)
Define structures with methods using the class and method macros. Use bind to attach block-closures to objects.
#include "oop.h"
#include <stdio.h>

// Define a Class
class(Person) {
    const char *name;
    method(void, talk, void); // Method definition
};

// Constructor
ctor(Person, const char *name) {
    this->name = name;
    
    // Bind a method using Clang Blocks!
    bind(this, talk, {
        printf("Hello, my name is %s!\n", this->name); 
    });
    return 0;
}

// Destructor
dtor(Person) {
    unbind(this, talk); // Automatically releases the copied block
}

int main() {
    // Instantiate using 'new'
    Person *p = new(Person, "Alice");
    
    // Call the method
    p->talk();
    
    // Clean up
    delete(Person, p);
    return 0;
}

2. RAII & Smart Pointers (sptr)
Tired of explicit and error-prone free() calls? Use sptr to declare a scope-bound smart pointer. It tracks reference counts under the hood and automatically triggers the destructor when it goes out of scope.
#include "raii.h"
#include "oop.h"
#include <stdio.h>

void scope_demo() {
    // Declare a smart pointer using 'sptr'
    // It automatically binds a cleanup attribute to the current scope.
    sptr sptr_t *p1 = new(sptr_t, new(Person, "Bob"), getdtor(Person));

    {
        // Borrow the pointer (Increments reference count)
        sptr sptr_t *p2 = p1->borrow(p1);
        
        // p2 is used here...
        
    } // p2 goes out of scope here -> refc decrements, but object stays alive.
    
    printf("Leaving scope_demo...\n");
} // p1 goes out of scope here -> refc hits 0 -> Person object and control blocks are automatically freed!

Architecture & Core Macros
oop.h
•	class(name): Syntactic sugar for defining explicit object blueprints.
•	method(ret, name, ...): Declares a Clang block-based method pointer.
•	bind(self, member, ...) / unbind(...): Manages the lifecycle of method closures using Block_copy and Block_release to prevent memory leaks.
•	new(name, ...) / delete(name, obj): Type-safe memory allocation wrappers that sequence constructors (_init) and destructors (_destroy).
raii.h
•	sptr: Leverages __attribute__((cleanup(fn))) to trigger deterministic destruction at the closing brace }.
•	sptr_t: A reference-counted control block that wraps generic void * pointers alongside custom deleter callbacks.
License
This project is licensed under the MIT License - feel free to forge, modify, and distribute it in both open-source and commercial applications!
