#ifndef PIC_H
#define PIC_H

#include <k/types.h>

void pic_remap(int offset1, int offset2);
void pic_send_eoi(u8 irq);

void pic_disable(void);
void irq_set_mask(u8 IRQline);
void irq_clear_mask(u8 IRQline);

#endif /* PIC_H */
