// TODO license

/******************************************************************************
 INCLUDES
 ******************************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <inttypes.h>

#include "bplib.h"
#include "bplib_os.h"

#include "mutex.h"
#include "rmutex.h"
#include "cond.h"
#include "ztimer.h"
#include "random.h"

/******************************************************************************
 DEFINES
 ******************************************************************************/

#define UNIX_SECS_AT_2000     946684800
#define BP_MAX_LOG_ENTRY_SIZE 256
#define BP_MAX_LOCKS          128

//#define BPLIB_MUTEX_DEBUG

/******************************************************************************
 TYPEDEFS
 ******************************************************************************/

typedef struct
{
    cond_t  cond;
    mutex_t mutex; // TODO confirm this needs to be a rmutex
} bplib_os_lock_t;

/******************************************************************************
 FILE DATA
 ******************************************************************************/

static bplib_os_lock_t *locks[BP_MAX_LOCKS] = {0};
static rmutex_t  lock_of_locks = RMUTEX_INIT;

static size_t current_memory_allocated = 0;
static size_t highest_memory_allocated = 0;

static uint32_t flag_log_enable = BP_FLAG_NONCOMPLIANT | BP_FLAG_DROPPED | BP_FLAG_BUNDLE_TOO_LARGE |
                                  BP_FLAG_UNKNOWNREC | BP_FLAG_INVALID_CIPHER_SUITEID |
                                  BP_FLAG_INVALID_BIB_RESULT_TYPE | BP_FLAG_INVALID_BIB_TARGET_TYPE |
                                  BP_FLAG_FAILED_TO_PARSE | BP_FLAG_API_ERROR;

/******************************************************************************
 EXPORTED UTILITY FUNCTIONS
 ******************************************************************************/

/*--------------------------------------------------------------------------------------
 * bplib_os_enable_log_flags -
 *-------------------------------------------------------------------------------------*/
void bplib_os_enable_log_flags(uint32_t enable_mask)
{
    flag_log_enable = enable_mask;
}

/******************************************************************************
 EXPORTED FUNCTIONS
 ******************************************************************************/

/*--------------------------------------------------------------------------------------
 * bplib_os_init -
 *-------------------------------------------------------------------------------------*/
void bplib_os_init(void)
{
    /* The mutex is initialized statically. Still called here again. */
    rmutex_init(&lock_of_locks);

    ztimer_acquire(ZTIMER_MSEC);
    ztimer_acquire(ZTIMER_SEC);

    // TODO check if/how generation algo can be changed from default
    /* RNG is seeded by inclusion of auto_init_random module */
    return;
}

/*--------------------------------------------------------------------------------------
 * bplib_os_log -
 *
 * Returns - the error code passed in (for convenience)
 *-------------------------------------------------------------------------------------*/
int bplib_os_log(const char *file, unsigned int line, uint32_t *flags, uint32_t event, const char *fmt, ...)
{
    if ((flag_log_enable & event) == event)
    {
        char    formatted_string[BP_MAX_LOG_ENTRY_SIZE];
        va_list args;
        int     vlen, msglen;

        /* Build Formatted String */
        va_start(args, fmt);
        vlen   = vsnprintf(formatted_string, BP_MAX_LOG_ENTRY_SIZE - 1, fmt, args);
        msglen = vlen < BP_MAX_LOG_ENTRY_SIZE - 1 ? vlen : BP_MAX_LOG_ENTRY_SIZE - 1;
        va_end(args);

        /* Log Message */
        if (msglen > 0)
        {
            char  log_message[BP_MAX_LOG_ENTRY_SIZE];
            char *pathptr;

            formatted_string[msglen] = '\0';

            /* Chop Path in Filename */
            pathptr = strrchr(file, '/');
            if (pathptr)
                pathptr++;
            else
                pathptr = (char *)file;

            /* Create Log Message */
            if (event != BP_FLAG_DIAGNOSTIC)
            {
                msglen = snprintf(log_message, BP_MAX_LOG_ENTRY_SIZE, "%s:%u:%08X:%s", pathptr, line, event,
                                  formatted_string);
            }
            else
            {
                msglen = snprintf(log_message, BP_MAX_LOG_ENTRY_SIZE, "%s:%u:%s", pathptr, line, formatted_string);
            }

            /* Provide Truncation Indicator */
            if (msglen > (BP_MAX_LOG_ENTRY_SIZE - 2))
            {
                log_message[BP_MAX_LOG_ENTRY_SIZE - 2] = '#';
            }

            /* Display Log Message */
            fputs(log_message, stderr);
        }
    }

    /* Set Event Flag and Return */
    if (event > 0)
    {
        if (flags)
            *flags |= event;
        return BP_ERROR;
    }
    else
    {
        return BP_SUCCESS;
    }
}

/*--------------------------------------------------------------------------------------
 * bplib_os_get_dtntime_ms - returns milliseconds since DTN epoch
 * this should be compatible with the BPv7 time definition
 *-------------------------------------------------------------------------------------*/
uint64_t bplib_os_get_dtntime_ms(void)
{
    // TODO use RTC when possible or dynamically provide some offset
    // TODO ztimer64 exists. Evaluate if this would be better
    /*
     * There is no guarantee in RIOT that a RTC is present. The only thing that exists
     * for sure is a clock since the beginning of the execution.
     */
    ztimer_now_t now = ztimer_now(ZTIMER_MSEC);
    return now;
}

/*--------------------------------------------------------------------------------------
 * bplib_os_systime - returns seconds
 *-------------------------------------------------------------------------------------*/
int bplib_os_systime(unsigned long *sysnow)
{
    /*
     * There is no guarantee in RIOT that a RTC is present. The only thing that exists
     * for sure is a clock since the beginning of the execution. Since this function is
     * unused as of right now this just returns exactly that. Thus also no effort will
     * be put into ztimer64 right now.
     */
    *sysnow = ztimer_now(ZTIMER_SEC);

    /* Return Status */
    return BP_SUCCESS;
}

/*--------------------------------------------------------------------------------------
 * bplib_os_sleep
 *-------------------------------------------------------------------------------------*/
void bplib_os_sleep(int seconds)
{
    /* Assuming that negative sleep means no sleep. Function seems unused. */
    if (seconds < 0) return;
    ztimer_sleep(ZTIMER_SEC, (uint32_t) seconds);
}

/*--------------------------------------------------------------------------------------
 * bplib_os_random -
 *-------------------------------------------------------------------------------------*/
uint32_t bplib_os_random(void)
{
    return random_uint32();
}

/*--------------------------------------------------------------------------------------
 * bplib_os_createlock -
 *-------------------------------------------------------------------------------------*/
bp_handle_t bplib_os_createlock(void)
{
    bp_handle_t handle = BP_INVALID_HANDLE;

    rmutex_lock(&lock_of_locks);
    {
        int i;
        for (i = 0; i < BP_MAX_LOCKS; i++)
        {
            if (locks[i] == NULL)
            {
                locks[i] = (bplib_os_lock_t *)bplib_os_calloc(sizeof(bplib_os_lock_t));
                if (locks[i])
                {
                    mutex_init(&locks[i]->mutex);
                    cond_init(&locks[i]->cond);

                    handle = bp_handle_from_serial(i, BPLIB_HANDLE_OS_BASE);
                    break;
                }
            }
        }
    }
    rmutex_unlock(&lock_of_locks);

    return handle;
}

/*--------------------------------------------------------------------------------------
 * bplib_os_destroylock -
 *-------------------------------------------------------------------------------------*/
void bplib_os_destroylock(bp_handle_t h)
{
    int handle = bp_handle_to_serial(h, BPLIB_HANDLE_OS_BASE);

    rmutex_lock(&lock_of_locks);
    {
        if (locks[handle])
        {
            /* As far as I can see destroying mutex/cond is not needed here */
            bplib_os_free(locks[handle]);
            locks[handle] = NULL;
        }
    }
    rmutex_unlock(&lock_of_locks);
}

/*--------------------------------------------------------------------------------------
 * bplib_os_lock -
 *-------------------------------------------------------------------------------------*/
void bplib_os_lock(bp_handle_t h)
{
    extern volatile kernel_pid_t sched_active_pid;
    int handle = bp_handle_to_serial(h, BPLIB_HANDLE_OS_BASE);

    mutex_lock(&locks[handle]->mutex);
#ifdef BPLIB_MUTEX_DEBUG
    printf("BP Thread [%u] Handle [%u] Mutex   locked\n", thread_getpid(), h.hdl);
#endif
}

/*--------------------------------------------------------------------------------------
 * bplib_os_unlock -
 *-------------------------------------------------------------------------------------*/
void bplib_os_unlock(bp_handle_t h)
{
    int handle = bp_handle_to_serial(h, BPLIB_HANDLE_OS_BASE);

    mutex_unlock(&locks[handle]->mutex);
#ifdef BPLIB_MUTEX_DEBUG
    printf("BP Thread [%u] Handle [%u] Mutex unlocked\n", thread_getpid(), h.hdl);
#endif
}

/*--------------------------------------------------------------------------------------
 * bplib_os_broadcast_signal_and_unlock -
 *-------------------------------------------------------------------------------------*/
void bplib_os_broadcast_signal_and_unlock(bp_handle_t h)
{
    bplib_os_lock_t *lock = locks[bp_handle_to_serial(h, BPLIB_HANDLE_OS_BASE)];

    cond_broadcast(&lock->cond);
    mutex_unlock(&lock->mutex);
#ifdef BPLIB_MUTEX_DEBUG
    printf("BP Thread [%u] Handle [%u] Mutex unlocked, Cond broadcast\n", thread_getpid(), h.hdl);
#endif
}

/*--------------------------------------------------------------------------------------
 * bplib_os_broadcast_signal -
 *-------------------------------------------------------------------------------------*/
void bplib_os_broadcast_signal(bp_handle_t h)
{
    bplib_os_lock_t *lock = locks[bp_handle_to_serial(h, BPLIB_HANDLE_OS_BASE)];

    cond_broadcast(&lock->cond);
#ifdef BPLIB_MUTEX_DEBUG
    printf("BP Thread [%u] Handle [%u]               , Cond broadcast\n", thread_getpid(), h.hdl);
#endif
}

/*--------------------------------------------------------------------------------------
 * bplib_os_signal -
 *-------------------------------------------------------------------------------------*/
void bplib_os_signal(bp_handle_t h)
{
    int handle = bp_handle_to_serial(h, BPLIB_HANDLE_OS_BASE);

    cond_signal(&locks[handle]->cond);
#ifdef BPLIB_MUTEX_DEBUG
    printf("BP Thread [%u] Handle [%u]               , Cond signal\n", thread_getpid(), h.hdl);
#endif
}

/*--------------------------------------------------------------------------------------
 * bplib_os_waiton -
 *-------------------------------------------------------------------------------------*/
int bplib_os_waiton(bp_handle_t h, int timeout_ms)
{
    int handle = bp_handle_to_serial(h, BPLIB_HANDLE_OS_BASE);
    int status;

    /* Perform Wait */
    if (timeout_ms == -1)
    {
        /* Block Forever until Success */
#ifdef BPLIB_MUTEX_DEBUG
        printf("BP Thread [%u] Handle [%u]               , Waiting INF\n", thread_getpid(), h.hdl);
#endif
        cond_wait(&locks[handle]->cond, &locks[handle]->mutex);
        /* RIOTs cond wait cannot explicitly fail */
        status = BP_SUCCESS;

    }
    else if (timeout_ms > 0)
    {
        /* Block on Timed Wait and Update Timeout */
#ifdef BPLIB_MUTEX_DEBUG
        printf("BP Thread [%u] Handle [%u]               , Waiting timed\n", thread_getpid(), h.hdl);
#endif
        status = ztimer_cond_wait_timeout(ZTIMER_MSEC, &locks[handle]->cond,
                                          &locks[handle]->mutex, timeout_ms);
        if (status == 0)
        {
            status = BP_SUCCESS;
        }
        else
        {
            status = BP_TIMEOUT;
        }
    }
    else /* timeout_ms = 0 */
    {
        /* conditional does not support a non-blocking attempt
         * so treat it as an immediate timeout */
        status = BP_TIMEOUT;
    }

    /* Return Status */
    return status;
}

/*--------------------------------------------------------------------------------------
 * bplib_os_wait_until_ms -
 *-------------------------------------------------------------------------------------*/
// int bplib_os_wait_until_ms(bp_handle_t h, uint64_t abs_dtntime_ms)
// {
//     bplib_os_lock_t *lock = locks[bp_handle_to_serial(h, BPLIB_HANDLE_OS_BASE)];
//     int              status;

//     if (abs_dtntime_ms == BP_DTNTIME_INFINITE)
//     {
//         /* Block Forever until Success */
// #ifdef BPLIB_MUTEX_DEBUG
//         printf("BP Thread [%u] Handle [%u]               , Waiting INF\n", thread_getpid(), h.hdl);
// #endif
//         cond_wait(&lock->cond, &lock->mutex);
//         status = 0;
//     }
//     else
//     {
//         uint64_t cur_dtn_time = bplib_os_get_dtntime_ms();
//         // TODO problems when timepoint already passed?

//         /* Block on Timed Wait and Update Timeout */
// #ifdef BPLIB_MUTEX_DEBUG
//         printf("BP Thread [%u] Handle [%u]               , Waiting timed\n", thread_getpid(), h.hdl);
// #endif
//         status = ztimer_cond_wait_timeout(ZTIMER_MSEC, &lock->cond, &lock->mutex, 
//                                           abs_dtntime_ms - cur_dtn_time);
//     }

//     /* check for timeout error explicitly and translate to BP_TIMEOUT */
//     if (status == 0)
//     {
//         return BP_SUCCESS;
//     }

//     /* RIOT implementation cannot return any other errors */
//     return BP_TIMEOUT;
// }

/*--------------------------------------------------------------------------------------
 * bplib_os_format -
 *-------------------------------------------------------------------------------------*/
int bplib_os_format(char *dst, size_t len, const char *fmt, ...)
{
    va_list args;
    int     vlen;

    /* Build Formatted String */
    va_start(args, fmt);
    vlen = vsnprintf(dst, len, fmt, args);
    va_end(args);

    /* Return Error Code */
    return vlen;
}

/*--------------------------------------------------------------------------------------
 * bplib_os_strnlen -
 *-------------------------------------------------------------------------------------*/
int bplib_os_strnlen(const char *str, int maxlen)
{
    int len;
    for (len = 0; len < maxlen; len++)
    {
        if (str[len] == '\0')
        {
            return len;
        }
    }
    return maxlen;
}

/*----------------------------------------------------------------------------
 * bplib_os_calloc
 *----------------------------------------------------------------------------*/
void *bplib_os_calloc(size_t size)
{
    /* Allocate Memory Block */
    size_t   block_size = size + sizeof(size_t);
    uint8_t *mem_ptr    = (uint8_t *)calloc(block_size, 1);
    if (mem_ptr)
    {
        /* Prepend Amount */
        size_t *size_ptr = (size_t *)mem_ptr;
        *size_ptr        = block_size;

        /* Update Statistics */
        current_memory_allocated += block_size;
        if (current_memory_allocated > highest_memory_allocated)
        {
            highest_memory_allocated = current_memory_allocated;
        }

        /* Return User Block */
        return (mem_ptr + sizeof(size_t));
    }
    else
    {
        return NULL;
    }
}

/*----------------------------------------------------------------------------
 * bplib_os_free
 *----------------------------------------------------------------------------*/
void bplib_os_free(void *ptr)
{
    if (ptr)
    {
        uint8_t *mem_ptr = (uint8_t *)ptr;

        /* Read Amount */
        size_t *size_ptr   = (size_t *)((uint8_t *)mem_ptr - sizeof(size_t));
        size_t  block_size = *size_ptr;

        /* Update Statistics */
        current_memory_allocated -= block_size;

        /* Free Memory Block */
        free(mem_ptr - sizeof(size_t));
    }
}

/*----------------------------------------------------------------------------
 * bplib_os_memused - how many bytes of memory currently allocated
 *----------------------------------------------------------------------------*/
size_t bplib_os_memused(void)
{
    return current_memory_allocated;
}

/*----------------------------------------------------------------------------
 * bplib_os_memhigh - the most total bytes in allocation at any given time
 *----------------------------------------------------------------------------*/
size_t bplib_os_memhigh(void)
{
    return highest_memory_allocated;
}