# myShell - A Custom Unix Shell Implementation

A lightweight, functional command-line shell written in C that mimics core features of bash/sh.

## Features

| Category | Features |
|----------|----------|
| **Basic Shell** | Prompt display, command parsing, fork/exec execution |
| **Built-in Commands** | `cd`, `exit`, `pwd`, `history` |
| **Process Management** | Foreground execution, background execution (`&`), PID display |
| **I/O Redirection** | Input (`<`), Output (`>`) |
| **Pipes** | Command chaining using `|` |
| **Signal Handling** | Ctrl+C kills foreground child only (shell stays alive) |
| **History Persistence** | Commands saved to `~/.myshell_history` and loaded on startup |

## Requirements

- GCC compiler (version 4.8 or later)
- Linux/Unix environment (or WSL on Windows)

## Compilation

```bash
make
```

## Usage

Run the shell:

```bash
./myShell
```

Clean build artifacts:

```bash
make clean
```

## Command Examples

### Basic Commands
```bash
myShell> ls -la
myShell> pwd
myShell> cd /home/user/Documents
myShell> echo "Hello World"
```

### Background Execution
```bash
myShell> sleep 30 &
[Background process 12345]
```

### I/O Redirection
```bash
myShell> ls -la > output.txt
myShell> cat < input.txt
myShell> echo "Hello" > file.txt
```

### Pipes
```bash
myShell> ls -la | grep ".c"
myShell> ps aux | grep "bash" | wc -l
```

### Built-in Commands
```bash
myShell> cd /tmp
myShell> pwd
/tmp
myShell> history
1  ls -la
2  pwd
3  cd /tmp
myShell> exit
```

### Signal Handling
```bash
myShell> sleep 100
^C                    # Terminates sleep, shell continues running
myShell>
```

## Project Structure

```
myShell/
├── myShell.c          # Main shell source code
├── Makefile           # Build automation
└── README.md          # Documentation
```

## Technical Details

### System Calls Used
- `fork()` - Process creation
- `execvp()` - Program execution
- `waitpid()` / `wait()` - Process synchronization
- `pipe()` - Inter-process communication
- `dup2()` - File descriptor duplication
- `open()` / `close()` - File operations
- `chdir()` - Directory change
- `getcwd()` - Working directory retrieval
- `signal()` / `sigaction()` - Signal handling
- `kill()` - Signal delivery

### Limits
| Parameter | Value |
|-----------|-------|
| Max command length | 1024 characters |
| Max arguments per command | 64 |
| Max pipeline commands | 2 |
| History capacity | 100 commands |

### Known Limitations
- No quoted argument support (arguments with spaces not handled)
- No environment variable expansion (`$HOME`)
- No wildcard expansion (`*.c`)
- No command substitution (`` `cmd` ``)

## Error Handling

The shell handles the following errors gracefully:
- Command not found
- File/directory access errors
- Fork/pipe failures
- Invalid redirection syntax

## License

MIT License

Copyright (c) 2026

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
