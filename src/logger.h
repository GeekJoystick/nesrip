#ifndef LOGGER_H
#define LOGGER_H

extern char* programName;

void findProgramName(char* programPath);
void printProgamName();
void printHelp();
void printNoInput();
void printInvalidArg(char* arg);
void printInvalidArgUsage(const char* arg, const char* error);
void printInvalidDatabaseCommand(const char* arg, const char* error);
#endif