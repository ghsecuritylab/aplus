# aPlus - Hardware Abstraction Layer


## Timer


    `typedef TYPE ktime_t`
    
    `ktime_t timer_gettimestamp()`
    `ktime_t timer_getticks()`
    `ktime_t timer_getms()`
    `ktime_t timer_getus()`
    `ktime_t timer_getfreq()`
    `void timer_delay(ktime_t milliseconds)`
---

## Task
    `typedef STRUCT ARCH_task_context`

    `volatile task_t* arch_task_clone(int (*fn) (void*), void* stack, int flags, void* arg)`
    `volatile task_t* arch_task_fork(void)`
    `void arch_task_yield(void)`
    `void arch_task_switch(volatile* task_t* prev_task, volatile task_t* new_task)`
    `void arch_task_release(volatile* task_t* task)`
---

## Memory
    `typedef POINTER physaddr_t`
    `typedef POINTER virtaddr_t`

    `physaddr_t __V2P(virtaddr_t virtaddr)`
    `void map_page(virtaddr_t virtaddr, physaddr_t physaddr, int user)`
    `void unmap_page(virtaddr_t virtaddr)`
    `virtaddr_t get_free_pages(int count, int maked, int user)`
---

## Interrupts
    `void intr_enable(void)`
    `void intr_disable(void)`
---

## ELF
    `int arch_elf_check_machine(Elf_Ehdr* elf)`
---

## Debug
    `void debug_send(char value)`
    `void debug_lookup_symbol(symbol_t* symtab, uintptr_t address)`
    `void debug_dump(void* context, char* errmsg, uintptr_t dump, uintptr_t errcode)`
---