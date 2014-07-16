// Copyright 2014 GordonLee
#pragma once 

class ILogger {
public:
    virtual void Debug(const char* text) = 0;
    virtual void Info(const char* text) = 0;
    virtual void Warn(const char* text) = 0;
    virtual void Error(const char* text) = 0;
    virtual void Fatal(const char* text) = 0;
};

