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

`-i` flag adds every subfolder's header files to the include path (similar to the -i flag in the gcc compiler). So, instead of writing:

```
#include "subfolder/another_folder/test.h"
```

you can simply write:

```
#include "test.h"
```