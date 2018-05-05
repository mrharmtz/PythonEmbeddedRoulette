author: mrharmtz

tested on windows using the mingw64 compiler, 
due to issues with mingw compiler and Python.h(https://bugs.python.org/issue11566) not playing nice with the Python.h, 
had to make a workaround.

there is also an issue when adding more than 4000 objects, 
for some reason it does not allow to decrease the ref count to all objects,
and because of it the program may end in 0xC0000005, which means not all memory was released