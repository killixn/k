.code32
.section .text

# Macro for interrupt handler without error
# (dummy val as error)
# from https://wiki.osdev.org/Interrupts_Tutorial#ISRs
.macro isr_no_err_stub num
.global isr_\num
isr_\num:
    pushl	$0
    pushl	$\num
    jmp	isr_common
.endm

# Macro for interrupt handler with error
.macro isr_err_stub num
.global isr_\num
isr_\num:
    pushl	$\num
    jmp	isr_common
.endm

# Interrupt Request Handlers
.macro irq_stub num, map
.global irq_\num
irq_\num:
    pushl	$0
    pushl	$\map
    jmp	isr_common
.endm


# Context Switching
# from https://k.lre.epita.fr/data/slides/kern-3.pdf
.global isr_common
isr_common:
  # <save registers>
  # pushes registers in this order :
  #     eax, ecx, edx, ebx, original esp, ebp, esi, edi
  pusha

  pushl %esp
  call irq_handler
  add $4, %esp

  # <restore registers>
  popa

  add $8, %esp
  iret

# read https://wiki.osdev.org/Interrupts_Tutorial#ISRs
isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_err_stub    8
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
isr_err_stub    13
isr_err_stub    14
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_no_err_stub 21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_err_stub 29
isr_err_stub 30
isr_no_err_stub 31
isr_no_err_stub 128

irq_stub 0, 32
irq_stub 1, 33
irq_stub 2, 34
irq_stub 3, 35
irq_stub 4, 36
irq_stub 5, 37
irq_stub 6, 38
irq_stub 7, 39
irq_stub 8, 40
irq_stub 9, 41
irq_stub 10, 42
irq_stub 11, 43
irq_stub 12, 44
irq_stub 13, 45
irq_stub 14, 46
irq_stub 15, 47
