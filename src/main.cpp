#ifdef _WIN32
#include <windows.h>
#include <string.h> // For strcmp
using namespace std;

#else
#include <sys/wait.h>
#include <unistd.h>
std::string getCurrentPath() {
    char buffer[PATH_MAX];
    if (getcwd(buffer, sizeof(buffer)) != nullptr) {
        return std::string(buffer);
    } else {
        perror("getcwd() error");
        return std::string();
    }
}
#endif
#include <string>
#include <iostream>
#include <vector>
#include <cstring>
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
#ifdef _WIN32
        if (!SetCurrentDirectory(args[1])) {
            std::cerr << "cd: could not change directory" << std::endl;
        }
#else
        if (chdir(args[1]) != 0) {
            perror("cd");
        }
#endif
    }
}

#ifdef _WIN32
// Function to execute commands on Windows using CreateProcess
void executeCommand(const std::vector<char*>& args) {
    // CreateProcess needs the command and arguments as a single string
    std::string command;
    for (char* arg : args) {
        if (arg != nullptr) {
            command += arg;
            command += " ";
        }
    }

    // Set up the process information structures
    STARTUPINFO startupInfo;
    PROCESS_INFORMATION processInfo;
    ZeroMemory(&startupInfo, sizeof(startupInfo));
    startupInfo.cb = sizeof(startupInfo);
    ZeroMemory(&processInfo, sizeof(processInfo));

    // Create the process
    if (!CreateProcess(nullptr, &command[0], nullptr, nullptr, false, 0, nullptr, nullptr, &startupInfo, &processInfo)) {
        std::cerr << "CreateProcess failed (" << GetLastError() << ")." << std::endl;
    } else {
        // Wait until the child process exits
        WaitForSingleObject(processInfo.hProcess, INFINITE);

        // Close process and thread handles
        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);
    }
}
#else
// Function to execute commands on Unix-like systems using fork and execvp
void executeCommand(const std::vector<char*>& args) {
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
#endif

string getCurrentPath() {
    char buffer[MAX_PATH];  // MAX_PATH is a constant defined by Windows
    GetCurrentDirectory(MAX_PATH, buffer);
    return string(buffer);
}

// Main shell loop
void shellLoop() {
    std::string input;
    while (true) {
        std::cout << "SHELL> " << getCurrentPath() << ">";    // Display shell prompt
        std::getline(std::cin, input);   // Read user input

        if (input.empty()) continue;  // Ignore empty input

        // Parse the input into command and arguments
        std::vector<char*> args = parseInput(input);

        if (strcmp(args[0], "exit") == 0) {
            break;   // Exit shell
        } else if (strcmp(args[0], "cd") == 0) {
            changeDirectory(args);    // Handle 'cd' command
        } else {
            // Execute other commands
            executeCommand(args);
        }
    }
}

int main() {
    shellLoop();    // Start the shell
    return 0;
}
