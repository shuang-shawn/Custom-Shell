Overview
This is a simple custom shell implementation in C. The shell provides basic functionality such as executing commands, changing directories (cd), managing background processes, and handling signals like SIGINT. The shell supports various execution modes including foreground, background, sequential foreground, and parallel foreground.

Features
Command Execution: Executes commands entered by the user, including built-in commands like cd.
Background Processes: Allows running processes in the background by appending & to the command.
Sequential and Parallel Execution: Supports sequential execution of commands separated by && and parallel execution using &&&.
Change Directory: Implements the cd command, including support for the ~ shortcut.
Signal Handling: Catches and handles SIGINT (Ctrl+C) to avoid termination of the shell.
Background Process Management: Tracks and manages background processes, including cleanup when a process finishes.
Foreground Process Management: Manages foreground processes and waits for them to finish when necessary.
Prerequisites
To compile and run the custom shell, ensure you have:

A C compiler (e.g., GCC)
Standard C libraries
A Unix-like environment (Linux or macOS)
Usage
Compilation: Compile the source code (shell.c) using your preferred C compiler. For example:

Copy code
gcc shell.c -o shell
Execution: Run the compiled executable:

bash
Copy code
./shell
Shell Commands:

Enter commands as you would in a standard shell environment.
Use exit to exit the shell.
Use cd to change directories.
Append & to a command to run it in the background.
Use && to execute commands sequentially.
Use &&& to execute commands in parallel.
Examples
Running a command in the foreground:

shell
Copy code
$ ls -l
Running a command in the background:

shell
Copy code
$ sleep 10 &
Changing directory:

shell
Copy code
$ cd /path/to/directory
Sequential execution of commands:

shell
Copy code
$ echo "Hello" && echo "World"
Parallel execution of commands:

shell
Copy code
$ echo "Hello" &&& echo "World"
