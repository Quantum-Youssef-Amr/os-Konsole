#include <stdio.h>    // Standard input/output functions (printf, perror, fgets)
#include <stdlib.h>   // Standard library functions (exit, malloc, free, getenv)
#include <string.h>   // String manipulation functions (strcpy, strtok, strlen)
#include <unistd.h>   // POSIX API (fork, execvp, chdir, getcwd, pipe, close)
#include <sys/wait.h> // Process waiting functions (waitpid, wait)
#include <fcntl.h>    // File control options (open, O_RDONLY, O_WRONLY)
#include <signal.h>   // Signal handling functions (signal, kill)

#define MAX_INPUT 1024  // Maximum length of user input command line
#define MAX_ARGS 64     // Maximum number of command arguments
#define MAX_HISTORY 100 // Maximum number of commands stored in history

char *history[MAX_HISTORY]; // Array of pointers to stored command strings
int history_count = 0;      // Current number of commands in history buffer
pid_t foreground_pid = -1;  // Process ID of currently running foreground child

// Function prototypes declared before main for proper compilation
void read_command(char *input);               // Reads user input from stdin
void parse_command(char *input, char **args); // Splits command into arguments
void execute_command(char *input);            // Main command execution dispatcher
void handle_sigint(int sig);                  // Signal handler for Ctrl+C
void add_to_history(char *command);           // Adds command to history buffer
void show_history(void);                      // Displays all stored commands
void save_history(void);                      // Writes history to disk file
void load_history(void);                      // Reads history from disk file

// Signal handler function for SIGINT (Ctrl+C keyboard interrupt)
void handle_sigint(int sig)
{
    (void)sig; // Suppress unused parameter compiler warning
    if (foreground_pid > 0)
    {                                 // Check if there is a foreground process
        kill(foreground_pid, SIGINT); // Send termination signal to child only
    }
    printf("\n");   // Print newline after signal interruption
    fflush(stdout); // Flush output buffer to display immediately
}

// Adds a new command to the circular history buffer
void add_to_history(char *command)
{
    if (command == NULL || strlen(command) == 0)
        return; // Ignore empty commands

    if (history_count < MAX_HISTORY)
    {                                             // If buffer has free space
        history[history_count] = strdup(command); // Duplicate and store command
        history_count++;                          // Increment history counter
    }
}

// Displays all previously executed commands with line numbers
void show_history(void)
{
    for (int i = 0; i < history_count; i++)
    {                                          // Loop through history array
        printf("%d  %s\n", i + 1, history[i]); // Print index and command
    }
}

// Persists command history to .myshell_history file in user's home directory
void save_history(void)
{
    char *home = getenv("HOME"); // Get home directory path
    if (home == NULL)
        return; // Exit if HOME not set

    char path[512];                                            // Buffer for full file path
    snprintf(path, sizeof(path), "%s/.myshell_history", home); // Build history file path

    FILE *file = fopen(path, "w"); // Open file for writing
    if (file == NULL)
        return; // Exit if cannot open file

    for (int i = 0; i < history_count; i++)
    {                                      // Loop through all commands
        fprintf(file, "%s\n", history[i]); // Write each command to file
        free(history[i]);                  // Free allocated memory
    }

    fclose(file); // Close file handle
}

// Loads previously saved command history from home directory at shell startup
void load_history(void)
{
    char *home = getenv("HOME"); // Get home directory path
    if (home == NULL)
        return; // Exit if HOME not set

    char path[512];                                            // Buffer for full file path
    snprintf(path, sizeof(path), "%s/.myshell_history", home); // Build history file path

    FILE *file = fopen(path, "r"); // Open file for reading
    if (file == NULL)
        return; // Exit if file doesn't exist

    char line[MAX_INPUT]; // Buffer for each line
    while (fgets(line, sizeof(line), file) != NULL)
    {                                     // Read line by line
        line[strcspn(line, "\n")] = '\0'; // Remove trailing newline
        if (history_count < MAX_HISTORY)
        {                                          // Check buffer capacity
            history[history_count] = strdup(line); // Duplicate and store line
            history_count++;                       // Increment counter
        }
    }

    fclose(file); // Close file handle
}

// Reads a single line of input from the user and displays the prompt
void read_command(char *input)
{
    printf("myShell> "); // Display shell prompt
    fflush(stdout);      // Force prompt to appear

    if (fgets(input, MAX_INPUT, stdin) == NULL)
    {                 // Read user input
        printf("\n"); // Print newline on EOF (Ctrl+D)
        exit(0);      // Exit shell gracefully
    }

    input[strcspn(input, "\n")] = '\0'; // Remove newline character
}

// Splits a command string into an array of arguments using whitespace delimiters
void parse_command(char *input, char **args)
{
    int i = 0;                          // Index for arguments array
    char *token = strtok(input, " \t"); // Get first token (split on space/tab)

    while (token != NULL && i < MAX_ARGS - 1)
    {                                // Loop until no more tokens
        args[i++] = token;           // Store token in arguments array
        token = strtok(NULL, " \t"); // Get next token
    }

    args[i] = NULL; // Null-terminate arguments array
}

// Checks if command is built-in and executes it internally without forking
int is_builtin(char **args)
{
    if (args[0] == NULL)
        return 0; // No command to check

    if (strcmp(args[0], "cd") == 0)
    { // Check for cd command
        if (args[1] == NULL)
        {                          // No directory specified
            chdir(getenv("HOME")); // Change to home directory
        }
        else
        { // Directory provided
            if (chdir(args[1]) != 0)
            {                 // Attempt directory change
                perror("cd"); // Print error if fails
            }
        }
        return 1; // Indicate command was handled
    }

    if (strcmp(args[0], "exit") == 0)
    {                   // Check for exit command
        save_history(); // Save history before exiting
        exit(0);        // Terminate shell process
    }

    if (strcmp(args[0], "pwd") == 0)
    {                        // Check for pwd command
        char cwd[MAX_INPUT]; // Buffer for current directory
        if (getcwd(cwd, sizeof(cwd)) != NULL)
        {                        // Get working directory
            printf("%s\n", cwd); // Print directory path
        }
        else
        {                  // getcwd failed
            perror("pwd"); // Print error message
        }
        return 1; // Indicate command was handled
    }

    if (strcmp(args[0], "history") == 0)
    {                   // Check for history command
        show_history(); // Display command history
        return 1;       // Indicate command was handled
    }

    return 0; // Not a built-in command
}

// Main command execution function handling pipes, redirection, background, and fork
void execute_command(char *input)
{
    char input_copy[MAX_INPUT]; // Mutable copy of input
    strcpy(input_copy, input);  // Copy to preserve original

    // Check for background execution (ampersand at end of command)
    int background = 0;           // Flag for background execution
    int len = strlen(input_copy); // Get command length
    if (len > 0 && input_copy[len - 1] == '&')
    {                               // If last char is ampersand
        background = 1;             // Set background flag
        input_copy[len - 1] = '\0'; // Remove ampersand from command
        while (len > 0 && input_copy[len - 1] == ' ')
        {                               // Trim trailing spaces
            input_copy[len - 1] = '\0'; // Remove space character
            len--;                      // Decrement length counter
        }
    }

    // Check for pipe operator between commands
    char *pipe_pos = strchr(input_copy, '|'); // Find pipe character
    if (pipe_pos != NULL)
    {                                   // Pipe found in command
        *pipe_pos = '\0';               // Split at pipe position
        char *left_cmd = input_copy;    // Left side of pipe
        char *right_cmd = pipe_pos + 1; // Right side of pipe

        while (*left_cmd == ' ')
            left_cmd++; // Trim left command spaces
        while (*right_cmd == ' ')
            right_cmd++; // Trim right command spaces

        int pipefd[2]; // File descriptors for pipe
        if (pipe(pipefd) == -1)
        {                   // Create pipe for communication
            perror("pipe"); // Print error if fails
            return;         // Exit function early
        }

        pid_t pid1 = fork(); // Create first child process
        if (pid1 == 0)
        {                                   // First child process code
            dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to pipe write end
            close(pipefd[0]);               // Close pipe read end (unused)
            close(pipefd[1]);               // Close original pipe write end

            char *args[MAX_ARGS];          // Arguments array for exec
            parse_command(left_cmd, args); // Parse left command

            if (!is_builtin(args))
            {                          // If not built-in command
                execvp(args[0], args); // Execute external program
                perror("execvp");      // Print error if exec fails
                exit(1);               // Exit child with failure
            }
            exit(0); // Exit child successfully
        }

        pid_t pid2 = fork(); // Create second child process
        if (pid2 == 0)
        {                                  // Second child process code
            dup2(pipefd[0], STDIN_FILENO); // Redirect stdin from pipe read end
            close(pipefd[0]);              // Close pipe read end
            close(pipefd[1]);              // Close pipe write end (unused)

            char *args[MAX_ARGS];           // Arguments array for exec
            parse_command(right_cmd, args); // Parse right command

            if (!is_builtin(args))
            {                          // If not built-in command
                execvp(args[0], args); // Execute external program
                perror("execvp");      // Print error if exec fails
                exit(1);               // Exit child with failure
            }
            exit(0); // Exit child successfully
        }

        // Parent process closes both pipe ends
        close(pipefd[0]); // Close pipe read end
        close(pipefd[1]); // Close pipe write end

        if (!background)
        {                           // Foreground pipeline execution
            waitpid(pid1, NULL, 0); // Wait for first child to complete
            waitpid(pid2, NULL, 0); // Wait for second child to complete
        }
        else
        {                                              // Background pipeline execution
            printf("[Background process %d]\n", pid1); // Print first background PID
            printf("[Background process %d]\n", pid2); // Print second background PID
        }

        return; // Pipeline handling complete
    }

    // Check for input redirection operator (<)
    char *input_redir = strchr(input_copy, '<'); // Find input redirection symbol
    int input_fd = -1;                           // File descriptor for input file
    char *input_file = NULL;                     // Name of input file

    if (input_redir != NULL)
    {                                 // Input redirection requested
        *input_redir = '\0';          // Split at redirection symbol
        input_file = input_redir + 1; // Get filename after symbol
        while (*input_file == ' ')
            input_file++; // Skip leading spaces

        input_fd = open(input_file, O_RDONLY); // Open file for reading only
        if (input_fd == -1)
        {                                // Check if open failed
            perror("input redirection"); // Print error message
            return;                      // Exit function early
        }
    }

    // Check for output redirection operator (>)
    char *output_redir = strchr(input_copy, '>'); // Find output redirection symbol
    int output_fd = -1;                           // File descriptor for output file
    char *output_file = NULL;                     // Name of output file

    if (output_redir != NULL)
    {                                   // Output redirection requested
        *output_redir = '\0';           // Split at redirection symbol
        output_file = output_redir + 1; // Get filename after symbol
        while (*output_file == ' ')
            output_file++; // Skip leading spaces

        output_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644); // Open/create file for writing
        if (output_fd == -1)
        {                                 // Check if open failed
            perror("output redirection"); // Print error message
            if (input_fd != -1)
                close(input_fd); // Close input file if open
            return;              // Exit function early
        }
    }

    // Parse the command into arguments array
    char *args[MAX_ARGS];            // Array for command arguments
    parse_command(input_copy, args); // Split command into tokens

    // Check if command is built-in (cd, exit, pwd, history)
    if (is_builtin(args))
    { // Built-in command detected
        if (input_fd != -1)
            close(input_fd); // Close input file if open
        if (output_fd != -1)
            close(output_fd); // Close output file if open
        return;               // Built-in already executed
    }

    // Fork a new process for external command execution
    pid_t pid = fork(); // Create child process

    if (pid == 0)
    { // Child process code
        if (input_fd != -1)
        {                                 // Input redirection active
            dup2(input_fd, STDIN_FILENO); // Redirect stdin from file
            close(input_fd);              // Close original file descriptor
        }

        if (output_fd != -1)
        {                                   // Output redirection active
            dup2(output_fd, STDOUT_FILENO); // Redirect stdout to file
            close(output_fd);               // Close original file descriptor
        }

        execvp(args[0], args); // Replace child with external program
        perror("execvp");      // Print error if exec fails
        exit(1);               // Exit child with failure status
    }
    else if (pid > 0)
    { // Parent process code
        if (input_fd != -1)
            close(input_fd); // Close input file in parent
        if (output_fd != -1)
            close(output_fd); // Close output file in parent

        if (!background)
        {                          // Foreground command execution
            foreground_pid = pid;  // Store PID for signal handling
            waitpid(pid, NULL, 0); // Wait for child to complete
            foreground_pid = -1;   // Reset foreground PID
        }
        else
        {                                             // Background command execution
            printf("[Background process %d]\n", pid); // Print background process ID
        }
    }
    else
    {                   // fork() returned -1 (error)
        perror("fork"); // Print fork error message
    }
}

// Main entry point of the shell program
int main(void)
{
    char input[MAX_INPUT]; // Buffer for user input

    signal(SIGINT, handle_sigint); // Register Ctrl+C signal handler

    load_history(); // Load saved commands from file

    while (1)
    {                        // Infinite shell loop
        read_command(input); // Get user input

        if (strlen(input) > 0)
        {                           // Non-empty command
            add_to_history(input);  // Store in history buffer
            execute_command(input); // Parse and execute command
        }
    }

    return 0; // Never reached (exit handled)
}