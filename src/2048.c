#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <conio.h>
#include <Windows.h>

#pragma comment(lib, "user32.lib")

#define POW(x) ((x) * (x))
#define LEN 4
#define INDEX(row_index, column_index) ((row_index)*LEN + (column_index))
#define MAX_NUM_LEN 6 // 2^17 == 131072

typedef uint_fast8_t uint_map_t;
typedef uint_fast32_t uint_score_t;
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

static const unsigned ColorArray[] = {239, 251, 67, 31, 51, 83, 21, 93, 164, 196, 226, 214};
static const size_t ColorArrayLength = sizeof(ColorArray) / sizeof(unsigned);

static uint_map_t map[POW(LEN)] = {0};
static uint_score_t score = 0;

void initialize(void);
void init_console(void);
void print_map(void);
Direction get_direction(void);
int modify_map(const Direction direction);
int modify_line(const size_t start, const size_t end, const int step);
size_t gen_number(void);
int check_game_over(void);

int main(void)
{
    extern uint_score_t score;

    initialize();

    /* 程序主循环 */
    for (int is_dead = 0; !is_dead;)
    {
        printf_s("\033c");     // 清屏
        printf_s("\x1b[?25l"); // 隐藏光标
        printf_s("\nScore: %u\n\n", score);
        print_map();

        while (!modify_map(get_direction()))
            ;

        if (!gen_number()) // 返回值：地图当前空位数
        {
            is_dead = check_game_over();
        }
    }

    printf_s("\033c");
    printf_s("\x1b[?25l");
    for (size_t i = 0; i < (MAX_NUM_LEN + 1) * LEN + 1; i++)
    {
        putchar('-');
    }
    putchar('\n');
    printf_s("Game over!\nFinal Score: %u\n", score);
    print_map();

    while (_getch() != '\x1b')
        ;

    return 0;
}

void initialize(void)
{
    extern uint_map_t map[POW(LEN)];

    /* 设置随机数种子 */
    srand((unsigned)time(NULL));

    /* 初始化控制台 */
    init_console();

    /* 设置控制台窗口标题 */
    printf_s("\x1b]0;2048\x7");

    /* 初始化地图 */
    int i = rand() % POW(LEN);
    int j = rand() % (POW(LEN) - 1);
    map[i] = 1;
    map[j + (i >= j)] = rand() % 10 ? 1 : 2;
}

void init_console(void)
{
    /* 获取标准输出流 */
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
    {
        puts("Error: GetStdHandle(STD_OUTPUT_HANDLE)");
        exit(EXIT_FAILURE);
    }

    /* 设置控制台信息 */
    CONSOLE_SCREEN_BUFFER_INFOEX ConsoleInfoEx;
    ConsoleInfoEx.cbSize = sizeof(ConsoleInfoEx);
    if (!GetConsoleScreenBufferInfoEx(hOut, &ConsoleInfoEx))
    {
        printf_s("Error: GetConsoleScreenBufferInfoEx (Code %lu)\n", GetLastError());
        exit(EXIT_FAILURE);
    }
    COORD ConsoleBufferSize = {
        .X = ((MAX_NUM_LEN + 1) * LEN) + 1,
        .Y = (2 * LEN + 1) + 3};
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

    /* 设置控制台输出模式 */
    DWORD dwOutputMode;
    if (!GetConsoleMode(hOut, &dwOutputMode))
    {
        printf_s("Error: GetConsoleMode (Code %lu)\n", GetLastError());
        exit(EXIT_FAILURE);
    }
    if (!SetConsoleMode(hOut, dwOutputMode | DISABLE_NEWLINE_AUTO_RETURN | ENABLE_VIRTUAL_TERMINAL_PROCESSING))
    {
        printf_s("Error: SetConsoleMode (Code %lu)\n", GetLastError());
        exit(EXIT_FAILURE);
    }

    /* 获取标准输入流 */
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hIn == INVALID_HANDLE_VALUE)
    {
        puts("Error: GetStdHandle(STD_INPUT_HANDLE)");
        exit(EXIT_FAILURE);
    }

    /* 设置控制台输入模式 */
    DWORD dwInputMode;
    if (!GetConsoleMode(hIn, &dwInputMode))
    {
        printf_s("Error: GetConsoleMode (Code %lu)\n", GetLastError());
        exit(EXIT_FAILURE);
    }
    if (!SetConsoleMode(hIn, dwInputMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING))
    {
        printf_s("Error: SetConsoleMode (Code %lu)\n", GetLastError());
        exit(EXIT_FAILURE);
    }

    /* 获取控制台窗口句柄 */
    HWND hConsoleWindow = GetConsoleWindow();
    if (hConsoleWindow == NULL)
    {
        puts("Error: GetConsoleWindow()");
        exit(EXIT_FAILURE);
    }

    /* 获取控制台窗口样式 */
    LONG_PTR WindowStyle = GetWindowLongPtr(hConsoleWindow, GWL_STYLE);
    if (!WindowStyle)
    {
        puts("Error: GetWindowLongPtr");
        exit(EXIT_FAILURE);
    }

    /* 设置控制台窗口样式 */
    if (!SetWindowLongPtr(hConsoleWindow,
                          GWL_STYLE,
                          WindowStyle & ~WS_HSCROLL & ~WS_HSCROLL & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX))
    {
        printf_s("Error: SetWindowLongPtr (Code %lu)\n", GetLastError());
        exit(EXIT_FAILURE);
    }
}

void print_map(void)
{
    extern uint_map_t map[POW(LEN)];

    extern const unsigned ColorArray[];
    extern const size_t ColorArrayLength;

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
        for (size_t column = 0, index; column < LEN; column++)
        {
            index = INDEX(row, column);
            putchar('|');
            printf_s("\x1b[38;5;%um%*u\x1b[0m",
                     map[index] > ColorArrayLength - 1 ? 7 : ColorArray[map[index]],
                     MAX_NUM_LEN,
                     map[index] ? 1 << map[index] : 0);
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

int modify_map(const Direction direction)
{
    int is_modified = 0;

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
        is_modified |= modify_line(current, current + ((LEN - 1) * step), step);
    }

    return is_modified;
}

int modify_line(const size_t start, const size_t end, const int step)
{
    extern uint_map_t map[POW(LEN)];
    extern uint_score_t score;

    int is_modified = 0;

    for (size_t slow_i = start, rest_of_part = 1;
         rest_of_part && slow_i != end;
         slow_i += step)
    {
        for (size_t fast_i = slow_i + step;
             fast_i != end + step;
             rest_of_part = ((fast_i += step) != (end + step)))
        {
            if (map[fast_i])
            {
                if (!map[slow_i])
                {
                    map[slow_i] = map[fast_i];
                    map[fast_i] = 0;
                    is_modified = 1;
                }
                else
                {
                    if (map[slow_i] == map[fast_i])
                    {
                        score += (uint_score_t)(1 << (++map[slow_i]));
                        map[fast_i] = 0;
                        is_modified = 1;
                    }
                    break;
                }
            }
        }
    }

    return is_modified;
}

size_t gen_number(void)
{
    extern uint_map_t map[POW(LEN)];

    /* 空位计数 */
    size_t empty_slot_num = 0;

    /* 扫描地图空位数 */
    for (size_t i = 0; i < POW(LEN); i++)
    {
        if (!map[i])
        {
            empty_slot_num++;
        }
    }

    /* 有空位时随机生成数字 */
    if (empty_slot_num)
    {
        for (size_t index = 0,
                    empty_slot_count = 0,
                    target = rand() % empty_slot_num + 1;
             ;
             index++)
        {
            // 如果当前索引处为空位
            if (!map[index])
            {
                empty_slot_count++; // 递增计数

                if (empty_slot_count == target)
                {
                    map[index] = rand() % 10 ? 1 : 2;
                    empty_slot_num--;
                    break;
                }
            }
        }
    }

    return empty_slot_num;
}

int check_game_over(void)
{
    extern uint_map_t map[POW(LEN)];

    for (size_t row = 0; row < LEN - 1; row++)
    {
        for (size_t column = 0; column < LEN - 1; column++)
        {
            if (map[INDEX(row, column)] == map[INDEX(row, column + 1)] ||
                map[INDEX(row, column)] == map[INDEX(row + 1, column)])
            {
                return 0;
            }
        }
    }

    for (size_t row = 0, column = LEN - 1; row < LEN - 1; row++)
    {
        if (map[INDEX(row, column)] == map[INDEX(row + 1, column)])
        {
            return 0;
        }
    }

    for (size_t row = LEN - 1, column = 0; column < LEN - 1; column++)
    {
        if (map[INDEX(row, column)] == map[INDEX(row, column + 1)])
        {
            return 0;
        }
    }

    return 1;
}