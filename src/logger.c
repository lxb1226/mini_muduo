/*
 * @Author: heyjude
 * @Date: 2022-03-26 20:13:49
 * @LastEditors: heyjude
 * @LastEditTime: 2022-03-26 20:34:58
 * @FilePath: /mini_muduo/src/logger.c
 * @Description:
 *
 * Copyright (c) 2022 by heyjude, All Rights Reserved.
 */

#include "include/logger.h"

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>

#define MAXLINE 256

#ifndef NDEBUG
int logLevel = LOG_LEVEL_DEBUG;
#else
int logLevel = LOG_LEVEL_INFO;
#endif

static int logFd = STDOUT_FILENO;

static const char *log_level_str[] = {
    "[TRACE]",
    "[DEBUG]",
    "[INFO] ",
    "[WARN] ",
    "[ERROR]",
    "[FATAL]"};

void setLevel(int level)
{
    if (level < LOG_LEVEL_TRACE)
        level = LOG_LEVEL_TRACE;
    if (level > LOG_LEVEL_FATAL)
        level = LOG_LEVEL_FATAL;
    logLevel = level;
}

void setFd(int fd)
{
    if (fd <= 0)
        fd = STDOUT_FILENO;
    logFd = fd;
}

// 日志格式： 时间 + 进程号 + 日志级别 + 日志内容 + 调用文件所在处 + 行号
// 时间：年月日 时:分:秒.微妙
void log_base(const char *file,
              int line,
              int level,
              int to_abort,
              const char *fmt, ...)
{
    char data[MAXLINE];
    // 获取时间
    struct timeval tv;
    time_t seconds = tv.tv_sec;

    gettimeofday(&tv, NULL);

    struct tm tm_time;

    gmtime_r(&seconds, &tm_time);

    size_t idx = snprintf(data, MAXLINE - 1, "%4d%02d%02d %02d:%02d:%02d.%06ld",
                       tm_time.tm_year + 1900, tm_time.tm_mon, tm_time.tm_mday,
                       tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, tv.tv_usec);
    // 获取进程号并写入
    idx += snprintf(data + idx, MAXLINE - idx, " [%d]", getpid());

    // 写入日志级别
    idx += snprintf(data + idx, MAXLINE - idx, " %s ", log_level_str[level]);

    // 写入日志内容
    va_list ap;
    va_start(ap, fmt);
    idx += vsnprintf(data + idx, MAXLINE - idx, fmt, ap);
    va_end(ap);

    // 写入调用文件名及行号
    int err = dprintf(logFd, "%s - %s:%d\n", data, strrchr(file, '/') + 1, line);
    if(err == -1){
        fprintf(stderr, "log failed");
    }

    if(to_abort){
        abort();
    }

}

void log_sys(const char *file,
             int line,
             int to_abort,
             const char *fmt, ...)
{
    char data[MAXLINE];
    // 获取时间
    struct timeval tv;
    time_t seconds = tv.tv_sec;

    gettimeofday(&tv, NULL);

    struct tm tm_time;

    gmtime_r(&seconds, &tm_time);

    int idx = snprintf(data, MAXLINE - 1, "%4d%02d%02d %02d:%02d:%02d.%06ld",
                       tm_time.tm_year + 1900, tm_time.tm_mon, tm_time.tm_mday,
                       tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, tv.tv_usec);
    // 获取进程号并写入
    idx += snprintf(data + idx, MAXLINE - idx, " [%d]", getpid());

    // 写入日志级别
    idx += snprintf(data + idx, MAXLINE - idx, " %s ", to_abort ? "[SYSFA]" : "[SYSER]");

    // 写入日志内容
    va_list ap;
    va_start(ap, fmt);
    idx += vsnprintf(data + idx, MAXLINE - idx, fmt, ap);
    va_end(ap);

    dprintf(logFd, "%s : %s - %s:%d\n", data, strerror(errno), strrchr(file, '/') + 1, line);
    if(to_abort){
        abort();
    }
}
