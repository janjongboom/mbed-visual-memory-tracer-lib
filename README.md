# Visual Memory Tracer for Mbed OS - embedded library

Embedded library to accompany [mbed-visual-memory-tracer](https://github.com/janjongboom/mbed-visual-memory-tracer).

## How to use

1. Add this library to your project through the Online Compiler, or via Mbed CLI:

    ```
    $ mbed add https://github.com/janjongboom/mbed-visual-memory-tracer-lib
    ```

1. Enable heap stats and memory tracing in your `mbed_app.json`:

    ```json
    {
        "macros": [
            "MBED_HEAP_STATS_ENABLED=1",
            "MBED_MEM_TRACING_ENABLED"
        ]
    }
    ```

1. Call the initialization functions from *both* `mbed_main` and `main`:

    ```cpp
    #include "mbed_vis_mem_tracer.h"

    extern "C" void mbed_main() {
        mbed_vismem_preinit();
    }

    int main() {
        mbed_vismem_init();

        // ... rest of your application
    }
    ```
