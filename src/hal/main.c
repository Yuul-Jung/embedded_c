/* main.c */
#include "hal.h"

void app_run(void);

int main(void)
{
    app_run();
    hal_report();
    return 0;
}
