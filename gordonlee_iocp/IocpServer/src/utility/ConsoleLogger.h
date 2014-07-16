// Copyright 2014 GordonLee
#pragma once 
#include "ILogger.h"
#include <stdio.h>

//MEMO: test logger. 
class ConsoleLogger : public ILogger {
public:
    virtual void Debug(const char* text) {
        Print("Debug", text);
    }

    virtual void Info(const char* text) {
        Print("Info", text);
    }

    virtual void Warn(const char* text) {
        Print("Warn", text);
    }

    virtual void Error(const char* text) {
        Print("Error", text);
    }

    virtual void Fatal(const char* text) {
        Print("Fatal", text);
    }

    static ConsoleLogger& getInstance() {
        static ConsoleLogger logger;
        return logger;
    }

private:
    void Print(const char* category, const char* text) {
        printf("[%s] - %s", category, text);
    }
};
