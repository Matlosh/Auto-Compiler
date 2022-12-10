# Auto Compiler
`Note: Auto Compiler is NOT any sorts of compiler to C, it just watches for change of any .c file and - if any change was made - it simply uses "gcc -o" command to compile the all files.`

---
In order to watch for a full project and for any changes in its .c files, this project's `main.c` has to be compiled. To do that simply use:
```
gcc main.c -o auto_compiler
```
or any command that lets user compile the .c file.

<br>

Next, compiled file should be moved to the root folder of the project that has to be watched over and created `auto_compiler.exe` needs to be opened there.

<br>

### Additional flags

`-I` flag adds every subfolder's header files to the include path (similar to the -I flag in the gcc compiler). So, instead of writing:

```
#include "subfolder/another_folder/test.h"
```

you can simply write:

```
#include "test.h"
```

<br>

`-T` flag allows to change default watch interval time (time period between comparing all files' modification date. (*Note*: Passed value is in milliseconds) For example:

```
auto_compiler main -T500
```

above simply changes watch interval time from the default 1000 milliseconds to the 500 milliseconds.

<br>

`-C` flag allows to append additional compilation flags (and similar) to every automatic gcc compilation command. Below:

```
auto_compiler main -C"-I"SDL\include\SDL2" -L"SDL\lib""
```

will lead to execution of the below command every time any of the given source files changes:

```
gcc ./main.c  -o main -ISDL\include\SDL2 -LSDL\lib
```