# Auto Compiler
`Note: Auto Compiler is NOT any sorts of compiler to C, it just watches for change of any .c file and - if any change was made - it simply uses "gcc -o" command to compile the all files.`

---
In order to watch full project for any of the .c file changes, this project's `main.c` has to be compiled. To do that simply use:
```
gcc main.c -o auto_compiler
```
or any command that lets user to compile the .c file.

<br>

Next, compiled file should be moved to the root folder of the project that has to be watched over and created `auto_compiler.exe` needs to be opened there.