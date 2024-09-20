#include <iostream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>

// Function to split the input into command and arguments
std::vector<char*> parseInput(std::string& input) {
    std::vector<char*> args;
    char* token = strtok(&input[0], " ");
    while (token != nullptr) {
        args.push_back(token);
        token = strtok(nullptr, " ");
    }
    args.push_back(nullptr);  // execvp requires a null-terminated array of args
    return args;
}

// Function to change directories (built-in 'cd' command)
void changeDirectory(const std::vector<char*>& args) {
    if (args.size() < 2) {
        std::cerr << "cd: expected argument" << std::endl;
    } else {
        if (chdir(args[1]) != 0) {
            perror("cd");
        }
    }
}

// Main shell loop
void shellLoop() {
    std::string input;
    while (true) {
        std::cout << "shell> ";    // Display shell prompt
        std::getline(std::cin, input);   // Read user input
        
        if (input.empty()) continue;  // Ignore empty input
        
        // Parse the input into command and arguments
        std::vector<char*> args = parseInput(input);
        
        if (strcmp(args[0], "exit") == 0) {
            break;   // Exit shell
        } else if (strcmp(args[0], "cd") == 0) {
            changeDirectory(args);    // Handle 'cd' command
        } else {
            // Fork a new process for other commands
            pid_t pid = fork();
            if (pid == 0) {
                // Child process: execute the command
                if (execvp(args[0], args.data()) == -1) {
                    perror("execvp");
                }
                exit(EXIT_FAILURE);
            } else if (pid < 0) {
                // Fork failed
                perror("fork");
            } else {
                // Parent process waits for the child to finish
                waitpid(pid, nullptr, 0);
            }
        }
    }
}

int main() {
    shellLoop();    // Start the shell
    return 0;
}