/*
 * test_buffer.c
 * -------------
 * Unit test / smoke test for SharedBuffer implementation.
 *
 * Verifies:
 *   1. Buffer initialization
 *   2. Enqueue operations
 *   3. Dequeue operations
 *   4. FIFO ordering
 *   5. Buffer full condition
 *   6. Buffer empty condition
 *   7. Buffer cleanup
 *
 * Build:
 *   gcc -Wall -Wextra -Werror -O2 -pthread \
 *    -Iinclude \
 *     tests/test_buffer.c \
 *     src/buffer/shared_buffer.c \
 *     -o build/test_buffer
 *
 * Run:
 *   ./build/test_buffer
 */

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "shared_buffer.h"

/* ---------------------------------------------------------
 * Helper: Create a dummy packet
 * --------------------------------------------------------- */
static DataUnit make_packet(int id, const char *source)
{
    DataUnit p;

    memset(&p, 0, sizeof(DataUnit));

    p.packet_id = id;

    strncpy(p.source,
            source,
            SOURCE_LEN - 1);

    snprintf(p.data,
             DATA_LEN,
             "DUMMY_PAYLOAD_FOR_PACKET_%d",
             id);

    p.size = (int)strlen(p.data);

    p.timestamp = time(NULL);

    return p;
}

/* ---------------------------------------------------------
 * Main Test Driver
 * --------------------------------------------------------- */
int main(void)
{
    SharedBuffer buf;

    DataUnit pkt;
    DataUnit out;

    int ret;

    printf("\n");
    printf("========================================\n");
    printf(" Shared Buffer Smoke Test\n");
    printf("========================================\n\n");

    /* =====================================================
     * Test 1: Initialization
     * ===================================================== */
    printf("[TEST 1] buffer_init()\n");

    ret = buffer_init(&buf);

    if (ret == BUF_OK &&
        buf.head == 0 &&
        buf.tail == 0 &&
        buf.count == 0)
    {
        printf("PASS\n\n");
    }
    else
    {
        printf("FAIL\n");
        return 1;
    }

    /* =====================================================
     * Test 2: Enqueue 3 Packets
     * ===================================================== */
    printf("[TEST 2] enqueue() x3\n");

    for (int i = 1; i <= 3; i++)
    {
        pkt = make_packet(i, "City-Hyderabad");

        ret = enqueue(&buf, &pkt);

        if (ret != BUF_OK)
        {
            printf("FAIL\n");
            return 1;
        }
    }

    if (buf.count == 3)
    {
        printf("PASS\n\n");
    }
    else
    {
        printf("FAIL\n");
        return 1;
    }

    /* =====================================================
     * Test 3: FIFO Verification
     * ===================================================== */
    printf("[TEST 3] dequeue() FIFO check\n");

    ret = dequeue(&buf, &out);

    if (ret == BUF_OK &&
        out.packet_id == 1)
    {
        printf("PASS\n\n");
    }
    else
    {
        printf("FAIL\n");
        return 1;
    }

    /* =====================================================
     * Test 4: Fill Buffer Completely
     * ===================================================== */
    printf("[TEST 4] Fill Buffer\n");

    for (int i = 4; i <= BUFFER_SIZE + 1; i++)
    {
        pkt = make_packet(i, "City-Chennai");

        ret = enqueue(&buf, &pkt);

        if (ret != BUF_OK)
        {
            printf("FAIL\n");
            return 1;
        }
    }

    if (buf.count == BUFFER_SIZE)
    {
        printf("PASS\n\n");
    }
    else
    {
        printf("FAIL\n");
        return 1;
    }

    /* =====================================================
     * Test 5: Buffer Full Condition
     * ===================================================== */
    printf("[TEST 5] enqueue() on FULL buffer\n");

    pkt = make_packet(999, "City-Mumbai");

    ret = enqueue(&buf, &pkt);

    if (ret == BUF_FULL)
    {
        printf("PASS\n\n");
    }
    else
    {
        printf("FAIL\n");
        return 1;
    }

    /* =====================================================
     * Test 6: Drain Buffer
     * ===================================================== */
    printf("[TEST 6] Drain Buffer\n");

    while (dequeue(&buf, &out) == BUF_OK)
    {
        /* drain everything */
    }

    if (buf.count == 0)
    {
        printf("PASS\n\n");
    }
    else
    {
        printf("FAIL\n");
        return 1;
    }

    /* =====================================================
     * Test 7: Buffer Empty Condition
     * ===================================================== */
    printf("[TEST 7] dequeue() on EMPTY buffer\n");

    ret = dequeue(&buf, &out);

    if (ret == BUF_EMPTY)
    {
        printf("PASS\n\n");
    }
    else
    {
        printf("FAIL\n");
        return 1;
    }

    /* =====================================================
     * Cleanup
     * ===================================================== */
    buffer_destroy(&buf);

    printf("========================================\n");
    printf(" ALL TESTS PASSED\n");
    printf("========================================\n\n");

    return 0;
}
