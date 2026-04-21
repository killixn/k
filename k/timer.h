#ifndef TIMER_H
#define TIMER_H

void init_timer();
unsigned long gettick(void);
void pit_irq_handler();

#endif /* TIMER_H */
