/* 기초 D 예제 검증용 (PC에서 컴파일·실행)
 * 빌드: gcc -std=c11 -Wall -Wextra -O0 test_func.c -o test_func
 */
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/* ---------- 기초 D-1: 함수 포인터 ---------- */

/* D-1. 함수 포인터 선언, 대입, 호출 */
typedef uint32_t (*binop_fn)(uint32_t, uint32_t);

static uint32_t add_u32(uint32_t a, uint32_t b) { return a + b; }
static uint32_t and_u32(uint32_t a, uint32_t b) { return a & b; }

static void demo_fnptr(void)
{
    binop_fn f = add_u32;                /* 함수 이름은 함수의 시작 주소로 쓰인다 */
    uint32_t r1 = f(0xF0u, 0x0Fu);       /* (*f)(...) 와 같은 호출 */

    f = and_u32;                         /* 같은 형의 다른 함수로 바꿔 끼운다 */
    uint32_t r2 = f(0xF0u, 0x3Cu);

    printf("[D-1] add=0x%02X and=0x%02X f==and_u32 ? %s\n",
           (unsigned)r1, (unsigned)r2, (f == and_u32) ? "참" : "거짓");
}

/* D-2. 함수 포인터 테이블로 명령 분기 */
struct command {
    const char *name;
    void (*run)(int arg);
};

static int g_led = 0;
static void cmd_on(int arg)  { (void)arg; g_led = 1; }
static void cmd_off(int arg) { (void)arg; g_led = 0; }
static void cmd_set(int arg) { g_led = arg; }

static const struct command cmd_table[] = {
    { "on",  cmd_on  },
    { "off", cmd_off },
    { "set", cmd_set },
};

static int dispatch(const char *name, int arg)
{
    for (size_t i = 0; i < sizeof cmd_table / sizeof cmd_table[0]; i++) {
        if (strcmp(cmd_table[i].name, name) == 0) {
            cmd_table[i].run(arg);
            return 0;
        }
    }
    return -1;                           /* 없는 명령 */
}

static void demo_table(void)
{
    int r_on  = dispatch("on", 0);
    int led_a = g_led;
    int r_set = dispatch("set", 5);
    int led_b = g_led;
    int r_bad = dispatch("zzz", 0);
    int r_off = dispatch("off", 0);

    printf("[D-2] on=%d led=%d / set=%d led=%d / zzz=%d / off=%d led=%d\n",
           r_on, led_a, r_set, led_b, r_bad, r_off, g_led);
}

/* ---------- 기초 D-2: 콜백 등록과 드라이버 인터페이스 ---------- */

/* D-3. 콜백 등록과 컨텍스트 전달 (ISR 등록의 원형) */
#define IRQ_COUNT 4

typedef void (*irq_handler_t)(void *ctx);

static struct {
    irq_handler_t handler;
    void         *ctx;
} irq_table[IRQ_COUNT];

static int irq_register(unsigned irq, irq_handler_t handler, void *ctx)
{
    if (irq >= IRQ_COUNT || handler == NULL) {
        return -1;
    }
    irq_table[irq].handler = handler;
    irq_table[irq].ctx = ctx;
    return 0;
}

static void irq_dispatch(unsigned irq)   /* 실제로는 인터럽트 진입 코드가 호출한다 */
{
    if (irq < IRQ_COUNT && irq_table[irq].handler != NULL) {
        irq_table[irq].handler(irq_table[irq].ctx);
    }
}

struct tick_state {
    uint32_t count;
};

static void on_timer(void *ctx)
{
    struct tick_state *st = (struct tick_state *)ctx;
    st->count++;
}

static void demo_irq(void)
{
    struct tick_state ts = {0};
    int r0 = irq_register(0, on_timer, &ts);
    int r1 = irq_register(9, on_timer, &ts);   /* 번호가 범위 밖 */

    irq_dispatch(0);
    irq_dispatch(0);
    irq_dispatch(2);                           /* 등록하지 않은 번호 */

    printf("[D-3] register(0)=%d register(9)=%d count=%u\n",
           r0, r1, (unsigned)ts.count);
}

/* D-4. 함수 포인터 구조체로 만든 드라이버 인터페이스 */
struct uart_ops {
    void (*putc)(char c);
    int  (*getc)(void);                        /* 받은 글자가 없으면 -1 */
};

static char   log_buf[16];
static size_t log_len;

static void mem_putc(char c)
{
    if (log_len < sizeof log_buf - 1) {
        log_buf[log_len++] = c;
        log_buf[log_len] = '\0';
    }
}
static int  mem_getc(void)      { return -1; }
static void stdout_putc(char c) { putchar(c); }
static int  stdout_getc(void)   { return -1; }

static const struct uart_ops ops_mem    = { mem_putc,    mem_getc    };
static const struct uart_ops ops_stdout = { stdout_putc, stdout_getc };

static void print_str(const struct uart_ops *ops, const char *s)
{
    while (*s != '\0') {
        ops->putc(*s++);
    }
}

static void demo_ops(void)
{
    print_str(&ops_mem, "HELLO");
    printf("[D-4] ops_mem 버퍼 = \"%s\" (%zu바이트)\n", log_buf, log_len);
    printf("      ops_stdout 출력 = ");
    print_str(&ops_stdout, "HELLO");
    putchar('\n');
}

int main(void)
{
    demo_fnptr();
    demo_table();
    demo_irq();
    demo_ops();
    return 0;
}
