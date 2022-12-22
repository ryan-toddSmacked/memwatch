

#include "memwatch.h"


#include <stdlib.h>
#include <ncurses.h>

int isLilEndian()
{
    volatile uint32_t i=0x01234567;
    // return 0 for big endian, 1 for little endian.
    return (*((uint8_t*)(&i))) == 0x67;
}


struct _Watcher
{
    void**  m_addresses;
    char*   m_adress_types;
    size_t  m_address_count;
};
typedef struct _Watcher wch_t;


static const char* m_str_types[12] = {
    "<CHR>:",
    "<I08>:",
    "<I16>:",
    "<I32>:",
    "<I64>:",
    "<U08>:",
    "<U16>:",
    "<U32>:",
    "<U64>:",
    "<F32>:",
    "<F64>:",
    "<ADR>:"
};




static WINDOW*   m_ncursesWindow = NULL;
static WINDOW*   m_std_scr       = NULL;
volatile wch_t   m_User          = {NULL, NULL, 0};
volatile bool    m_isInit        = false;

static const int   m_OS_voidptr_sz = sizeof(void*);
static const char* m_fmt = sizeof(void*) == 8 ? ("0x%016lX%s") : ("0x08X%s");

void printAddr(size_t rowi)
{
    if (m_User.m_addresses[rowi] == NULL)
        return;
    uint8_t* a = m_User.m_addresses[rowi];
    uintptr_t asUint = (uintptr_t)a;
    const char* typestr = m_str_types[m_User.m_adress_types[rowi]];
    int x = 1;
    mvwprintw(m_ncursesWindow, rowi + 2, x, m_fmt, asUint, typestr);
    x += sizeof(void*) * 2 + 8;

    const int dtype = m_User.m_adress_types[rowi];

    switch (dtype)
    {
        case watch_asChar:
            mvwprintw(m_ncursesWindow, rowi + 2, x, " %c", *(char*)a);
            break;
        case watch_asInt8:
            mvwprintw(m_ncursesWindow, rowi + 2, x, " %" PRId8, *(int8_t*)a);
            break;
        case watch_asInt16:
            mvwprintw(m_ncursesWindow, rowi + 2, x, " %" PRId16, *(int16_t*)a);
            break;
        case watch_asInt32:
            mvwprintw(m_ncursesWindow, rowi + 2, x, " %" PRId32, *(int32_t*)a);
            break;
        case watch_asInt64:
            mvwprintw(m_ncursesWindow, rowi + 2, x, " %" PRId64, *(int64_t*)a);
            break;
        case watch_asUint8:
            mvwprintw(m_ncursesWindow, rowi + 2, x, " %" PRIu8, *a);
            break;
        case watch_asUint16:
            mvwprintw(m_ncursesWindow, rowi + 2, x, " %" PRIu16, *(uint16_t*)a);
            break;
        case watch_asUint32:
            mvwprintw(m_ncursesWindow, rowi + 2, x, " %" PRIu32, *(uint32_t*)a);
            break;
        case watch_asUint64:
            mvwprintw(m_ncursesWindow, rowi + 2, x, " %" PRIu64, *(uint64_t*)a);
            break;
        case watch_asFloat:
            mvwprintw(m_ncursesWindow, rowi + 2, x, " %.7e", *(float*)a);
            break;
        case watch_asDouble:
            mvwprintw(m_ncursesWindow, rowi + 2, x, " %.15e", *(double*)a);
            break;
        case watch_asPtr:
            if (isLilEndian())
            {
                for (size_t i = 0; i < sizeof(void*); i++)
                {
                    mvwprintw(m_ncursesWindow, rowi + 2, x, " %02X", a[sizeof(void*) - i - 1]);
                    x += 3;
                }
            }
            else
            {
                for (size_t i = 0; i < sizeof(void*); i++)
                {
                    mvwprintw(m_ncursesWindow, rowi + 2, x, " %02X", a[i]);
                    x += 3;
                }
            }
            break;
        default:
            break;
    }
}


void watch_printf(const char* fmt, ...)
{
    if (!m_isInit)
        return;
    va_list args;
    va_start(args, fmt);
    vw_printw(m_std_scr, fmt, args);
    va_end(args);
    wrefresh(m_std_scr);
}

void watch_init(void)
{
    if (m_isInit)
        return;
    if (sizeof(void*) < 4)
        return;

    initscr();

    const int OS_ptr_char_width = m_OS_voidptr_sz * 2 + 8;
    const int memory_byte_width = 24;
    const int memory_char_width = OS_ptr_char_width + memory_byte_width + 2;

    if (memory_char_width >= COLS)
    {
        endwin();
        return;
    }

    m_ncursesWindow = newwin(LINES - 1, memory_char_width, 0, COLS - memory_char_width);

    m_User.m_addresses = calloc(LINES - 4, sizeof(void*));
    m_User.m_adress_types = calloc(LINES - 4, sizeof(char));
    m_User.m_address_count = LINES - 4;
    
    mvwprintw(m_ncursesWindow, 1, 1, "Memory Window:");
    box(m_ncursesWindow, 0, 0);
    wrefresh(m_ncursesWindow);

    m_std_scr = newwin(LINES - 1, COLS - memory_char_width - 1, 0, 0);
    scrollok(m_std_scr, true);
    wrefresh(m_std_scr);

    m_isInit = true;
}


void watch_end(void)
{
    if (!m_isInit)
        return;

    delwin(m_ncursesWindow);
    delwin(m_std_scr);
    endwin();

    free(m_User.m_addresses);
    free(m_User.m_adress_types);
}

size_t watch_howMany(void)
{
    return m_User.m_address_count;
}


void watch_this(const void* addr, size_t specificRow, dtype_t type)
{
    if (specificRow >= m_User.m_address_count)
        return;
    m_User.m_addresses[specificRow] = (void*)addr;
    m_User.m_adress_types[specificRow] = type;
}


void watch_notThis(size_t row)
{
    if (row >= m_User.m_address_count)
        return;
    m_User.m_addresses[row] = NULL;
}


void watch_refresh(void)
{
    if (!m_isInit)
        return;

    wclear(m_ncursesWindow);
    mvwprintw(m_ncursesWindow, 1, 1, "Memory Window:");
    box(m_ncursesWindow, 0, 0);
    for (size_t i = 0; i < m_User.m_address_count; i++)
        printAddr(i);
    wrefresh(m_ncursesWindow);
}

