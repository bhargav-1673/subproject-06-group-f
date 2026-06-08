#include "shared_buffer.h"

int main()
{
    SharedBuffer buffer;

    buffer_init(&buffer);

    /*
     * Producer Threads
     */

    /*
     * Consumer Threads
     */

    buffer_destroy(&buffer);

    return 0;
}