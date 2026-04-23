#ifndef LOGGER_ABSTRACT_H
#define LOGGER_ABSTRACT_H

class Logger {
    public:
        virtual ~Logger() {}
    
        virtual void LogDebugEx(const char* tag, const char* file, int line, const char* func, const char* fmt, ...) = 0;
        virtual void LogInfoEx(const char* tag, const char* file, int line, const char* func, const char* fmt, ...) = 0;
        virtual void LogErrorEx(const char* tag, const char* file, int line, const char* func, const char* fmt, ...) = 0;
        virtual void LogCheckError(int err, const char* file, int line, const char* func) = 0;
};
        
#endif
