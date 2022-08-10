#include <stdio.h>
#include <stdarg.h>

#define LEN 8

static unsigned line[LEN] = {0};
static unsigned score = 0;

void test(unsigned serial, ...);
void move(void);
void merge(void);

int main(void)
{
    unsigned serial = 0;

    test(serial, 0, 0, 0, 0, 0, 0, 0, 0);
    serial++;
    test(serial, 2, 0, 0, 0, 0, 0, 0, 0);
    serial++;
    test(serial, 0, 0, 0, 2, 0, 0, 0, 0);
    serial++;
    test(serial, 0, 0, 0, 0, 0, 0, 0, 2);
    serial++;
    test(serial, 2, 0, 4, 0, 8, 0, 4, 0);
    serial++;
    test(serial, 2, 2, 0, 0, 0, 0, 0, 0);
    serial++;
    test(serial, 2, 0, 2, 0, 0, 0, 0, 0);
    serial++;
    test(serial, 2, 0, 0, 0, 0, 0, 0, 2);
    serial++;
    test(serial, 2, 0, 2, 0, 0, 2, 0, 0);
    serial++;
    test(serial, 2, 0, 2, 0, 0, 4, 0, 0);
    serial++;
    test(serial, 2, 0, 2, 0, 0, 2, 0, 2);
    serial++;

    return 0;
}

void move(void)
{
    extern unsigned line[LEN];

    for (unsigned *slow = &line[0], *fast = slow + 1, isNotAllZero; slow != &line[LEN - 1]; slow++)
    {
        for (isNotAllZero = 1; *slow == 0 && isNotAllZero;)
        {
            for (fast = slow + 1, isNotAllZero = 0; fast != &line[LEN]; fast++)
            {
                isNotAllZero = isNotAllZero || *fast;
                *(fast - 1) = *fast;
                *fast = 0;
            }
        }
    }
}

void merge(void)
{
    extern unsigned line[LEN];

    for (unsigned *slow = &line[0], *fast = slow + 1; slow != &line[LEN - 1]; slow++, fast++)
    {
        if (*slow == *fast)
        {
            score += *slow;
            *slow *= 2;
            *fast = 0;
        }
    }
}

void test(unsigned serial, ...)
{
    extern unsigned line[LEN];

    va_list argList;
    va_start(argList, serial);

    printf("Situation %u:\n", serial);

    for (size_t i = 0; i < LEN; i++)
    {
        line[i] = (unsigned)va_arg(argList, int);
    }

    printf("before -> [");
    for (size_t i = 0; i < LEN; i++)
    {
        printf("%u", line[i]);

        if (i != LEN - 1)
        {
            printf("%s", ", ");
        }
    }
    puts("]");

    move();
    merge();
    move();

    printf("after  -> [");
    for (size_t i = 0; i < LEN; i++)
    {
        printf("%u", line[i]);

        if (i != LEN - 1)
        {
            printf("%s", ", ");
        }
    }
    puts("]\n");

    for (size_t i = 0; i < LEN; i++)
    {
        line[i] = 0;
    }
}