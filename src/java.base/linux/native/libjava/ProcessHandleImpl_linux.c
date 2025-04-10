/*
 * Copyright (c) 2015, 2025, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Oracle designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Oracle in the LICENSE file that accompanied this code.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */

#include "jni.h"
#include "jni_util.h"
#include "java_lang_ProcessHandleImpl.h"
#include "java_lang_ProcessHandleImpl_Info.h"

#include "ProcessHandleImpl_unix.h"

#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <string.h>
#include <ctype.h>

/*
 * Implementation of native ProcessHandleImpl functions for Linux.
 * See ProcessHandleImpl_unix.c for more details.
 */

/* Signatures for internal OS specific functions. */
static long long getBoottime(JNIEnv *env);

/* A static offset in milliseconds since boot. */
static long long bootTime_ms;
static long clock_ticks_per_second;
static int pageSize;

void os_initNative(JNIEnv *env, jclass clazz) {
    bootTime_ms = getBoottime(env);
    clock_ticks_per_second = sysconf(_SC_CLK_TCK);
    pageSize = sysconf(_SC_PAGESIZE);
}

/*
 * Return pids of active processes, and optionally parent pids and
 * start times for each process.
 * For a specific non-zero pid, only the direct children are returned.
 * If the pid is zero, all active processes are returned.
 * Reads /proc and accumulates any process following the rules above.
 * The resulting pids are stored into an array of longs named jarray.
 * The number of pids is returned if they all fit.
 * If the parentArray is non-null, store also the parent pid.
 * In this case the parentArray must have the same length as the result pid array.
 * Of course in the case of a given non-zero pid all entries in the parentArray
 * will contain this pid, so this array does only make sense in the case of a given
 * zero pid.
 * If the jstimesArray is non-null, store also the start time of the pid.
 * In this case the jstimesArray must have the same length as the result pid array.
 * If the array(s) (is|are) too short, excess pids are not stored and
 * the desired length is returned.
 */
jint os_getChildren(JNIEnv *env, jlong jpid, jlongArray jarray,
                    jlongArray jparentArray, jlongArray jstimesArray) {
    DIR* dir;
    struct dirent* ptr;
    pid_t pid = (pid_t) jpid;
    jlong* pids = NULL;
    jlong* ppids = NULL;
    jlong* stimes = NULL;
    jsize parentArraySize = 0;
    jsize arraySize = 0;
    jsize stimesSize = 0;
    jsize count = 0;

    arraySize = (*env)->GetArrayLength(env, jarray);
    JNU_CHECK_EXCEPTION_RETURN(env, -1);
    if (jparentArray != NULL) {
        parentArraySize = (*env)->GetArrayLength(env, jparentArray);
        JNU_CHECK_EXCEPTION_RETURN(env, -1);

        if (arraySize != parentArraySize) {
            JNU_ThrowIllegalArgumentException(env, "array sizes not equal");
            return 0;
        }
    }
    if (jstimesArray != NULL) {
        stimesSize = (*env)->GetArrayLength(env, jstimesArray);
        JNU_CHECK_EXCEPTION_RETURN(env, -1);

        if (arraySize != stimesSize) {
            JNU_ThrowIllegalArgumentException(env, "array sizes not equal");
            return 0;
        }
    }

    /*
     * To locate the children we scan /proc looking for files that have a
     * position integer as a filename.
     */
    if ((dir = opendir("/proc")) == NULL) {
        JNU_ThrowByNameWithMessageAndLastError(env,
            "java/lang/RuntimeException", "Unable to open /proc");
        return -1;
    }

    do { // Block to break out of on Exception
        pids = (*env)->GetLongArrayElements(env, jarray, NULL);
        if (pids == NULL) {
            break;
        }
        if (jparentArray != NULL) {
            ppids  = (*env)->GetLongArrayElements(env, jparentArray, NULL);
            if (ppids == NULL) {
                break;
            }
        }
        if (jstimesArray != NULL) {
            stimes  = (*env)->GetLongArrayElements(env, jstimesArray, NULL);
            if (stimes == NULL) {
                break;
            }
        }

        while ((ptr = readdir(dir)) != NULL) {
            pid_t ppid = 0;
            jlong totalTime = 0L;
            jlong startTime = 0L;

            /* skip files that aren't numbers */
            pid_t childpid = (pid_t) atoi(ptr->d_name);
            if ((int) childpid <= 0) {
                continue;
            }

            // Get the parent pid, and start time
            ppid = os_getParentPidAndTimings(env, childpid, &totalTime, &startTime);
            if (ppid >= 0 && (pid == 0 || ppid == pid)) {
                if (count < arraySize) {
                    // Only store if it fits
                    pids[count] = (jlong) childpid;

                    if (ppids != NULL) {
                        // Store the parentPid
                        ppids[count] = (jlong) ppid;
                    }
                    if (stimes != NULL) {
                        // Store the process start time
                        stimes[count] = startTime;
                    }
                }
                count++; // Count to tabulate size needed
            }
        }
    } while (0);

    if (pids != NULL) {
        (*env)->ReleaseLongArrayElements(env, jarray, pids, 0);
    }
    if (ppids != NULL) {
        (*env)->ReleaseLongArrayElements(env, jparentArray, ppids, 0);
    }
    if (stimes != NULL) {
        (*env)->ReleaseLongArrayElements(env, jstimesArray, stimes, 0);
    }

    closedir(dir);
    // If more pids than array had size for; count will be greater than array size
    return count;
}

/**
 * Read /proc/<pid>/stat and return the ppid, total cputime and start time.
 * -1 is fail;  >=  0 is parent pid
 * 'total' will contain the running time of 'pid' in nanoseconds.
 * 'start' will contain the start time of 'pid' in milliseconds since epoch.
 */
pid_t os_getParentPidAndTimings(JNIEnv *env, pid_t pid,
                                jlong *totalTime, jlong* startTime) {
    FILE* fp;
    char buffer[2048];
    int statlen;
    char fn[32];
    char* s;
    int parentPid;
    long unsigned int utime = 0;      // clock tics
    long unsigned int stime = 0;      // clock tics
    long long unsigned int start = 0; // microseconds

    /*
     * Try to stat and then open /proc/%d/stat
     */
    snprintf(fn, sizeof fn, "/proc/%d/stat", pid);

    fp = fopen(fn, "r");
    if (fp == NULL) {
        return -1;              // fail, no such /proc/pid/stat
    }

    /*
     * The format is: pid (command) state ppid ...
     * As the command could be anything we must find the right most
     * ")" and then skip the white spaces that follow it.
     */
    statlen = fread(buffer, 1, (sizeof buffer - 1), fp);
    fclose(fp);
    if (statlen < 0) {
        return -1;               // parent pid is not available
    }

    buffer[statlen] = '\0';
    s = strchr(buffer, '(');
    if (s == NULL) {
        return -1;               // parent pid is not available
    }
    // Found start of command, skip to end
    s++;
    s = strrchr(s, ')');
    if (s == NULL) {
        return -1;               // parent pid is not available
    }
    s++;

    // Scan the needed fields from status, retaining only ppid(4),
    // utime (14), stime(15), starttime(22)
    if (4 != sscanf(s, " %*c %d %*d %*d %*d %*d %*d %*u %*u %*u %*u %lu %lu %*d %*d %*d %*d %*d %*d %llu",
            &parentPid, &utime, &stime, &start)) {
        return 0;              // not all values parsed; return error
    }

    *totalTime = (utime + stime) * (jlong)(1000000000 / clock_ticks_per_second);

    *startTime = bootTime_ms + ((start * 1000) / clock_ticks_per_second);

    return parentPid;
}

void os_getCmdlineAndUserInfo(JNIEnv *env, jobject jinfo, pid_t pid) {
    int fd;
    int cmdlen = 0;
    char *cmdline = NULL, *cmdEnd = NULL; // used for command line args and exe
    char *args = NULL;
    jstring cmdexe = NULL;
    char fn[32];
    struct stat stat_buf;

    /*
     * Stat /proc/<pid> to get the user id
     */
    snprintf(fn, sizeof fn, "/proc/%d", pid);
    if (stat(fn, &stat_buf) == 0) {
        unix_getUserInfo(env, jinfo, stat_buf.st_uid);
        JNU_CHECK_EXCEPTION(env);
    }

    /*
     * Try to open /proc/<pid>/cmdline
     */
    strncat(fn, "/cmdline", sizeof fn - strnlen(fn, sizeof fn) - 1);
    if ((fd = open(fn, O_RDONLY)) < 0) {
        return;
    }

    do {                // Block to break out of on errors
        int i, truncated = 0;
        int count;
        char *s;

        /*
         * The path name read by readlink() is limited to PATH_MAX characters.
         * The content of /proc/<pid>/cmdline is limited to PAGE_SIZE characters.
         */
        cmdline = (char*)malloc((PATH_MAX > pageSize ? PATH_MAX : pageSize) + 1);
        if (cmdline == NULL) {
            break;
        }

        /*
         * On Linux, the full path to the executable command is the link in
         * /proc/<pid>/exe. But it is only readable for processes we own.
         */
        snprintf(fn, sizeof fn, "/proc/%d/exe", pid);
        if ((cmdlen = readlink(fn, cmdline, PATH_MAX)) > 0) {
            // null terminate and create String to store for command
            cmdline[cmdlen] = '\0';
            cmdexe = JNU_NewStringPlatform(env, cmdline);
            (*env)->ExceptionClear(env);        // unconditionally clear any exception
        }

        /*
         * The command-line arguments appear as a set of strings separated by
         * null bytes ('\0'), with a further null byte after the last
         * string. The last string is only null terminated if the whole command
         * line is not exceeding (PAGE_SIZE - 1) characters.
         */
        cmdlen = 0;
        s = cmdline;
        while ((count = read(fd, s, pageSize - cmdlen)) > 0) {
            cmdlen += count;
            s += count;
        }
        if (count < 0) {
            break;
        }
        // We have to null-terminate because the process may have changed argv[]
        // or because the content in /proc/<pid>/cmdline is truncated.
        cmdline[cmdlen] = '\0';
        if (cmdlen == pageSize && cmdline[pageSize - 1] != '\0') {
            truncated = 1;
        } else if (cmdlen == 0) {
            // /proc/<pid>/cmdline was empty. This usually happens for kernel processes
            // like '[kthreadd]'. We could try to read /proc/<pid>/comm in the future.
        }
        if (cmdlen > 0 && (cmdexe == NULL || truncated)) {
            // We have no exact command or the arguments are truncated.
            // In this case we save the command line from /proc/<pid>/cmdline.
            args = (char*)malloc(pageSize + 1);
            if (args != NULL) {
                memcpy(args, cmdline, cmdlen + 1);
                for (i = 0; i < cmdlen; i++) {
                    if (args[i] == '\0') {
                        args[i] = ' ';
                    }
                }
            }
        }
        i = 0;
        if (!truncated) {
            // Count the arguments
            cmdEnd = &cmdline[cmdlen];
            for (s = cmdline; *s != '\0' && (s < cmdEnd); i++) {
                s += strnlen(s, (cmdEnd - s)) + 1;
            }
        }
        unix_fillArgArray(env, jinfo, i, cmdline, cmdEnd, cmdexe, args);
    } while (0);

    if (cmdline != NULL) {
        free(cmdline);
    }
    if (args != NULL) {
        free(args);
    }
    if (fd >= 0) {
        close(fd);
    }
}

/**
 * Read the boottime from /proc/stat.
 */
static long long getBoottime(JNIEnv *env) {
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    long long bootTime = 0;

    fp = fopen("/proc/stat", "r");
    if (fp == NULL) {
        return -1;
    }

    while (getline(&line, &len, fp) != -1) {
        if (sscanf(line, "btime %llu", &bootTime) == 1) {
            break;
        }
    }
    free(line);

    if (fp != 0) {
        fclose(fp);
    }

    return bootTime * 1000;
}
