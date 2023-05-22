# C++ Virtual Methods Example

This example tests the usage of classes with virtual methods in C++.

Without the usage of `__libc_init_array`, the virtual table of the class instance `Example` is not initialized correctly:

```
vtable for 'ExampleClass' @ 0x0 (subobject @ 0x20000004):
[0]: 0x11e0006f
[1]: 0x0 <InterruptVectorDefault()>
```

In this case, a call to a virtual method will result to an invalid call
and the program won't work as expected.
Here, the program will only print out `Begin example`, but does not print any values.
It seems like the MCU resets here.

In other environments like with the Arduino Core, the hard fault handler is called
and the program gets into an infinite loop.

With the macro `CPLUSPLUS`, an implemention auf `__libc_init_array` is called on startup.

Then the virtual table is initialized correctly and new values are printed as expected:

```
vtable for 'ExampleClass' @ 0x212c (subobject @ 0x20000004):
[0]: 0x1fb0 <ExampleClass::doNewLine()>
[1]: 0x1fda <ExampleClass::printValue(int)>
```