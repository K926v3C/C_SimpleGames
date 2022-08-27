#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <conio.h>
#include <Windows.h>

#define POW(x) ((x) * (x))
#define LEN 4
#define MAX_NUM_LEN 6 // 2^17 == 131072

typedef uint_fast8_t map_uint;
typedef uint_fast32_t score_uint;
typedef struct
{
    unsigned changed_map : 1;
    unsigned is_filled : POW(LEN);
} map_status;
typedef enum
{
    UP,
    DOWN,
    LEFT,
    RIGHT
} Direction;
typedef enum
{
    TOP_LEFT_INDEX = 0,
    TOP_RIGHT_INDEX = (LEN - 1),
    BOTTOM_LEFT_INDEX = (LEN * (LEN - 1)),
    BOTTOM_RIGHT_INDEX = (LEN * LEN - 1)
} Corner;

static map_uint map[POW(LEN)] = {0};
static score_uint score = 0;
static map_status status = {.changed_map = 0, .is_filled = 0};

inline void init_console(void);
Direction get_direction(void);
void oprate_map(const Direction direction);
void move_line(const size_t start, const size_t end, const int step);
void merge_line(const size_t start, const size_t end, const int step);
inline void print_map(void);

int main(void)
{
    extern map_uint map[POW(LEN)];
    extern score_uint score;
    extern map_status status;

    /* 设置随机数种子 */
    srand((unsigned)time(NULL));

    /* 初始化控制台 */
    init_console();

    /* 空位计数 */
    size_t empty_slot_num = 0;

    /* 程序主循环 */
    for (;;)
    {
        /* 扫描地图填充状态 */
        for (size_t i = 0; i < POW(LEN); i++)
        {
            if (map[i])
            {
                status.is_filled |= (1 << i);
            }
            else
            {
                empty_slot_num++;
            }
        }

        /* 有空位时随机生成数字 */
        if (empty_slot_num)
        {
            for (size_t map_index = 0,
                        empty_slot_count = 0,
                        target_empty_slot = rand() % empty_slot_num + 1;
                 ;
                 map_index++)
            {
                // 如果当前索引处为空位
                if (!(status.is_filled & (1 << map_index)))
                {
                    empty_slot_count++; // 递增计数

                    if (empty_slot_count == target_empty_slot)
                    {
                        map[map_index] = rand() % 2 + 1;
                        break;
                    }
                }
            }
        }

        /* 显示地图并相应玩家操作 */
        print_map();
    change_map:
        oprate_map(get_direction());
        if (!status.changed_map)
        {
            goto change_map;
        }

        /* 重置状态信息 */
        status.changed_map = 0;
        status.is_filled = 0;
        empty_slot_num = 0;
    }

    /*
    TODO:
    - 颜色
    */

    return 0;
}

inline void init_console(void)
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
    {
        puts("Error: GetStdHandle()");
        exit(EXIT_FAILURE);
    }

    CONSOLE_SCREEN_BUFFER_INFOEX ConsoleInfoEx;
    ConsoleInfoEx.cbSize = sizeof(ConsoleInfoEx);
    if (!GetConsoleScreenBufferInfoEx(hOut, &ConsoleInfoEx))
    {
        printf_s("Error: GetConsoleScreenBufferInfoEx (Code %lu)\n", GetLastError());
        exit(EXIT_FAILURE);
    }

    COORD ConsoleBufferSize = {
        .X = ((MAX_NUM_LEN + 1) * LEN) + 1,
        .Y = (2 * LEN + 1) + 2};
    SMALL_RECT ConsoleWindowRect = {
        .Top = 0,
        .Left = 0,
        .Right = ConsoleBufferSize.X - 1,
        .Bottom = ConsoleBufferSize.Y};
    ConsoleInfoEx.dwSize = ConsoleBufferSize;
    ConsoleInfoEx.srWindow = ConsoleWindowRect;
    ConsoleInfoEx.bFullscreenSupported = FALSE;
    if (!SetConsoleScreenBufferInfoEx(hOut, &ConsoleInfoEx))
    {
        printf_s("Error: SetConsoleScreenBufferInfoEx (Code %lu)\n", GetLastError());
        exit(EXIT_FAILURE);
    }

    DWORD dwOriginalMode;
    if (!GetConsoleMode(hOut, &dwOriginalMode))
    {
        printf_s("Error: GetConsoleMode (Code %lu)\n", GetLastError());
        exit(EXIT_FAILURE);
    }

    if (!SetConsoleMode(hOut, dwOriginalMode | DISABLE_NEWLINE_AUTO_RETURN | ENABLE_VIRTUAL_TERMINAL_PROCESSING))
    {
        printf_s("Error: SetConsoleMode (Code %lu)\n", GetLastError());
        exit(EXIT_FAILURE);
    }

    /* 隐藏光标 */
    // putchar('\x1b');
    // printf_s("\x1b[?25l");
}

Direction get_direction(void)
{
    for (int key = -1;;)
    {
        key = _getch();

        switch (key)
        {
        case 0:
            switch (_getch())
            {
            case 72:
                goto up;

            case 75:
                goto left;

            case 77:
                goto right;

            case 80:
                goto down;

            default:
                continue;
            }

        case 'W':
        case 'w':
        up:
            return UP;

        case 'S':
        case 's':
        down:
            return DOWN;

        case 'A':
        case 'a':
        left:
            return LEFT;

        case 'D':
        case 'd':
        right:
            return RIGHT;

        default:
            continue;
        }
    }
}

void oprate_map(const Direction direction)
{
    size_t first, last;
    int step;

    switch (direction)
    {
    case UP:
        first = TOP_LEFT_INDEX;
        last = TOP_RIGHT_INDEX;
        step = LEN;
        break;

    case DOWN:
        first = BOTTOM_LEFT_INDEX;
        last = BOTTOM_RIGHT_INDEX;
        step = -LEN;
        break;

    case LEFT:
        first = TOP_LEFT_INDEX;
        last = BOTTOM_LEFT_INDEX;
        step = 1;
        break;

    case RIGHT:
        first = TOP_RIGHT_INDEX;
        last = BOTTOM_RIGHT_INDEX;
        step = -1;
        break;

    default:
        exit(EXIT_FAILURE);
    }

    // last - first > 0
    for (size_t current = first,
                i = (last - first) / (LEN - 1);
         current != last + i;
         current += i)
    {
        move_line(current, current + ((LEN - 1) * step), step);
        merge_line(current, current + ((LEN - 1) * step), step);
        move_line(current, current + ((LEN - 1) * step), step);
    }
}

void move_line(const size_t start, const size_t end, const int step)
{
    extern map_uint map[POW(LEN)];
    extern map_status status;

    size_t slow_i = start, fast_i;
    map_uint rest_of_part;

    do
    {
        for (fast_i = slow_i + step, rest_of_part = 0;
             fast_i != end + step; // fast_i 超出行尾时结束循环
             fast_i += step)
        {
            rest_of_part |= map[fast_i];

            if (!map[fast_i - step] && map[fast_i])
            {
                map[fast_i - step] = map[fast_i];
                map[fast_i] = 0;
                status.changed_map = 1;
            }
        }
        slow_i += map[slow_i] ? step : 0;
    } while (rest_of_part && slow_i != end);
}

void merge_line(const size_t start, const size_t end, const int step)
{
    extern map_uint map[POW(LEN)];
    extern score_uint score;
    extern map_status status;

    // i 指针触及行尾时即可结束循环
    for (size_t i = start; i != end; i += step)
    {
        if (map[i] && map[i] == map[i + step])
        {
            map[i + step] = 0;
            score += (1 << (++map[i]));
            status.changed_map = 1;
        }
    }
}

inline void print_map(void)
{
    extern map_uint map[POW(LEN)];
    extern score_uint score;

    /* 清屏 */
    putchar('\x1b');
    putchar('c');

    printf_s("Score: %u\n\n", score);

    for (size_t row = 0; row < LEN; row++)
    {
        /* 打印水平分隔线 */
        for (size_t column = 0; column < LEN; column++)
        {
            putchar('+');
            for (size_t i = 0; i < MAX_NUM_LEN; i++)
            {
                putchar('-');
            }
        }
        putchar('+');
        putchar('\n');

        /* 打印竖直分隔线与数字 */
        for (size_t column = 0; column < LEN; column++)
        {
            putchar('|');
            printf_s("%*u",
                     MAX_NUM_LEN,
                     map[row * LEN + column] ? 1 << map[row * LEN + column] : 0);
        }
        putchar('|');
        putchar('\n');
    }

    /* 打印水平分隔线 */
    for (size_t column = 0; column < LEN; column++)
    {
        putchar('+');
        for (size_t i = 0; i < MAX_NUM_LEN; i++)
        {
            putchar('-');
        }
    }
    putchar('+');
}