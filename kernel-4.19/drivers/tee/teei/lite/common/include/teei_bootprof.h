#ifndef TEEI_BOOTPROF_H
#define TEEI_BOOTPROF_H

extern void bootprof_log_boot(char *str);
#define TEEI_BOOT_FOOTPRINT(str) bootprof_log_boot(str)

#endif
