#include <windows.h>
#include <string.h> // For strcmp
#include <iostream>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <direct.h>
using namespace std;

// Function to split the input into command and arguments
vector<char*> parseInput(string& input) {
    vector<char*> args;
    char* token = strtok(&input[0], " ");
    while (token != nullptr) {
        args.push_back(token);
        token = strtok(nullptr, " ");
    }
    args.push_back(nullptr);  // Null-terminate the args array
    return args;
}

// Function to get the current directory
string getCurrentPath() {
    char buffer[MAX_PATH];
    if (GetCurrentDirectoryA(MAX_PATH, buffer)) {
        return string(buffer);  // Return narrow string
    } else {
        cerr << "Error getting current directory. Error code: " << GetLastError() << endl;
        return string();
    }
}

// Function to change directories (built-in 'cd' command)
void changeDirectory(const vector<char*>& args) {
    if (args.size() <= 2) {
        cout << "\033[33mcd: expected argument\033[0m" << endl;
    } else {
        if (_chdir(args[1]) != 0) {
            perror("cd");
        }
    }
}

// Function to print arguments for debugging
void print_arg(const vector<char*>& args) {
    int i;
    cout << "print(" << args.size() << ") : ";
    for (i = 0; i < args.size() - 1; i++) {
        cout << args[i] << " ";
    }
    cout << endl;
}

void executeCommand(const vector<char*>& args) {
    // Build command line string for CreateProcessA
    string cmdLine;
    for (int i = 0; i < args.size() - 1; i++) {
        if (strcmp(args[0], "ls") == 0){
            cmdLine += "dir ";
            continue;
        }
        cmdLine += string(args[i]) + " ";
    }
    
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Check if the command is a built-in command (like 'dir', 'copy', etc.)
    bool isBuiltinCommand = (strcmp(args[0], "dir") == 0 || 
                             strcmp(args[0], "ls") == 0 ||
                             strcmp(args[0], "copy") == 0 ||
                             strcmp(args[0], "del") == 0 ||
                             strcmp(args[0], "cls") == 0 ||
                             strcmp(args[0], "type") == 0);

    // Special handling for deleting directories with `del`
    if (strcmp(args[0], "del") == 0 && args.size() > 1) {
        // Check if the target is a directory
        DWORD attributes = GetFileAttributesA(args[1]);
        if (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY)) {
            // If it's a directory, switch to "rmdir /s /q" (recursive delete without confirmation)
            cmdLine = "cmd /c rmdir /s /q " + string(args[1]);
        }
    }

    if (isBuiltinCommand && cmdLine.find("rmdir") == string::npos) {
        // Prepend "cmd /c" to the command for normal built-in commands
        cmdLine = "cmd /c " + cmdLine;
    }

    // Convert the command line to a C-style string
    char* cmdLineCStr = new char[cmdLine.length() + 1];
    strcpy(cmdLineCStr, cmdLine.c_str());

    // Create a new process for the command
    if (!CreateProcessA(
        NULL,           // No module name (use command line)
        cmdLineCStr,    // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's current directory
        &si,            // Pointer to STARTUPINFO structure
        &pi             // Pointer to PROCESS_INFORMATION structure
    )) {
        cerr << "\033[31mCreateProcess failed. Error code: " << GetLastError() << "\033[0m" << endl;
    } else {
        // Wait for the command to finish executing
        WaitForSingleObject(pi.hProcess, INFINITE);

        // Close process and thread handles
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    delete[] cmdLineCStr;  // Free the allocated memory
}

// Function to create a new directory (built-in 'mkdir' command)
void makeDirectory(const vector<char*>& args) {
    if (args.size() <= 2) {
        cout << "\033[33mmkdir: expected argument\033[0m" << endl;
    } else {
        // _mkdir returns 0 on success and -1 on failure
        if (_mkdir(args[1]) == 0) {
            cout << "Directory created: " << args[1] << endl;
        } else {
            perror("mkdir");  // Prints error message if directory creation fails
        }
    }
}

// Main shell loop
void shellLoop() {
    string input;
    while (true) {
        cout << "\033[32mSHELL>\033[0m " << getCurrentPath() << "> ";    // Display shell prompt
        getline(cin, input);   // Read user input

        if (input.empty()) continue;  // Ignore empty input

        // Parse the input into command and arguments
        vector<char*> args = parseInput(input);
        
        if (strcmp(args[0], "exit") == 0) {
            system("cls");
            break;   // Exit shell
        } else if (strcmp(args[0], "cd") == 0) {
            changeDirectory(args);    // Handle 'cd' command
        } else if (strcmp(args[0], "mkdir") == 0) { 
            makeDirectory(args);
        } else {
            executeCommand(args);     // Handle external commands
        }
    }
}

int main() {
    system("cls");
    shellLoop();    // Start the shell
    return 0;
}
