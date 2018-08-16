#ifndef __VIS_MEM_TRACE_H__
#define __VIS_MEM_TRACE_H__

#include "mbed.h"
#include "mbed_mem_trace.h"
#include "mbed_stats.h"

#if !defined(MBED_MEM_TRACING_ENABLED) || !defined(MBED_HEAP_STATS_ENABLED)
#warning "VisMemTrace requires the MBED_MEM_TRACING_ENABLED and MBED_HEAP_STATS_ENABLED macros to be set"
#endif

#ifndef VISMEMTRACE_BUFFER_SIZE
#define VISMEMTRACE_BUFFER_SIZE     4096
#endif

static char*    vismem_preinit_orig_buffer;     // Pointer where the original buffer is declared
static char*    vismem_preinit_buffer;          // Current pointer of the buffer (used to store allocations)

static void mbed_vismem_trace_preinit_callback(uint8_t op, void *res, void *caller, ...)
{
    // If we could not allocate the preinit buffer, exit
    if (!vismem_preinit_buffer) {
        return;
    }

    int32_t left_in_buffer = VISMEMTRACE_BUFFER_SIZE - static_cast<int32_t>(vismem_preinit_buffer - vismem_preinit_orig_buffer);
    if (left_in_buffer <= 0) return;

    va_list va;
    size_t temp_s1, temp_s2;
    void *temp_ptr;

    int n;

    va_start(va, caller);
    switch (op) {
        case MBED_MEM_TRACE_MALLOC:
            temp_s1 = va_arg(va, size_t);
            n = snprintf(vismem_preinit_buffer, left_in_buffer, MBED_MEM_DEFAULT_TRACER_PREFIX "m:%p;%p-%u\r\n", res, caller, temp_s1);
            break;

        case MBED_MEM_TRACE_REALLOC:
            temp_ptr = va_arg(va, void *);
            temp_s1 = va_arg(va, size_t);
            n = snprintf(vismem_preinit_buffer, left_in_buffer, MBED_MEM_DEFAULT_TRACER_PREFIX "r:%p;%p-%p;%u\r\n", res, caller, temp_ptr, temp_s1);
            break;

        case MBED_MEM_TRACE_CALLOC:
            temp_s1 = va_arg(va, size_t);
            temp_s2 = va_arg(va, size_t);
            n = snprintf(vismem_preinit_buffer, left_in_buffer, MBED_MEM_DEFAULT_TRACER_PREFIX "c:%p;%p-%u;%u\r\n", res, caller, temp_s1, temp_s2);
            break;

        case MBED_MEM_TRACE_FREE:
            temp_ptr = va_arg(va, void *);
            n = snprintf(vismem_preinit_buffer, left_in_buffer, MBED_MEM_DEFAULT_TRACER_PREFIX "f:%p;%p-%p\r\n", res, caller, temp_ptr);
            break;

        default:
            n = 0;
            break;
    }
    va_end(va);

    // Notice that only when this returned value is non-negative and less than n, the string has been completely written.
    if (n > 0 && n < left_in_buffer) {
        vismem_preinit_buffer += n;
    }
}

static void mbed_vismem_trace_postinit_callback(uint8_t op, void *res, void *caller, ...)
{
    va_list va;
    size_t temp_s1, temp_s2;
    void *temp_ptr;

    va_start(va, caller);
    switch (op) {
        case MBED_MEM_TRACE_MALLOC:
            temp_s1 = va_arg(va, size_t);
            printf(MBED_MEM_DEFAULT_TRACER_PREFIX "m:%p;%p-%u\r\n", res, caller, temp_s1);
            break;

        case MBED_MEM_TRACE_REALLOC:
            temp_ptr = va_arg(va, void *);
            temp_s1 = va_arg(va, size_t);
            printf(MBED_MEM_DEFAULT_TRACER_PREFIX "r:%p;%p-%p;%u\r\n", res, caller, temp_ptr, temp_s1);
            break;

        case MBED_MEM_TRACE_CALLOC:
            temp_s1 = va_arg(va, size_t);
            temp_s2 = va_arg(va, size_t);
            printf(MBED_MEM_DEFAULT_TRACER_PREFIX "c:%p;%p-%u;%u\r\n", res, caller, temp_s1, temp_s2);
            break;

        case MBED_MEM_TRACE_FREE:
            temp_ptr = va_arg(va, void *);
            printf(MBED_MEM_DEFAULT_TRACER_PREFIX "f:%p;%p-%p\r\n", res, caller, temp_ptr);
            break;

        default:
            printf(MBED_MEM_DEFAULT_TRACER_PREFIX "?\r\n");
            break;
    }
    va_end(va);
}

/**
 * Initialize a buffer to store data before the serial interface is declared.
 *
 * Call this function from your `mbed_main` function:
 *
 *      extern "C" void mbed_main() {
 *          mbed_vismem_preinit();
 *      }
 */
void mbed_vismem_preinit()
{
    // don't run this in the callback, that will cause a never-ending loop
    vismem_preinit_orig_buffer = (char*)calloc(VISMEMTRACE_BUFFER_SIZE, 1);
    vismem_preinit_buffer = vismem_preinit_orig_buffer;

    // set the callback to the preinit function
    mbed_mem_trace_set_callback(mbed_vismem_trace_preinit_callback);
}

/**
 * Initialize the memory tracer, and flush the preinit buffer.
 *
 * Call this function from your `main` function - after intializing your serial interface:
 *
 *      int main() {
 *          // ... optional: initialize serial interface
 *
 *          mbed_vismem_init();
 *
 *          // ...rest of the application
 *      }
 */
void mbed_vismem_init()
{
    mbed_stats_heap_t heap_stats;
    mbed_stats_heap_get(&heap_stats);
    printf("#visual-memory-tracer-init:%lu:%lu\r\n", heap_stats.current_size, heap_stats.reserved_size);

    if (vismem_preinit_orig_buffer) {
        printf(vismem_preinit_orig_buffer);
        free(vismem_preinit_orig_buffer);
    }
    else {
        printf("#vismem buffer was not allocated\r\n");
    }

    // set the callback to the preinit function
    mbed_mem_trace_set_callback(mbed_vismem_trace_postinit_callback);
}

#endif // __VIS_MEM_TRACE_H__
