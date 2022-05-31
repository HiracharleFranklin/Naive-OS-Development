/**
 * @file paging.h
 * @brief Defines paging-related data structure and functions
 * @version 0.1
 * @date 2022-03-17
 */

#ifndef _PAGING_H
#define _PAGING_H

#include "types.h"
#include "lib.h"

typedef union page_directory_entry {
    uint32_t val;
    struct KByte {
        uint32_t present            : 1;    // When the flag is set, the page is in physical memory.
        uint32_t read_or_write      : 1;    // When the flag is set, the page can be read and written into.
        uint32_t user_or_supervisor : 1;    // When this flag is clear, the page is assigned the supervisor privilege level.
        uint32_t write_through      : 1;    // 1 indicates write-through caching is enabled, otherwise write-back caching is enabled
        uint32_t cache_disabled     : 1;    // 1 indicates the caching of individual pages or page tables is disabled
        uint32_t accessed           : 1;
        uint32_t reserved           : 1;    // set to 0
        uint32_t page_size          : 1;    // 0 indicates 4 KBytes
        uint32_t global_page        : 1;    // ignored
        uint32_t avail              : 3;    // available for system programmer’s use
        uint32_t base_address       : 20;   // page table base address
    } KByte __attribute__ ((packed));
    struct MByte {
        uint32_t present            : 1;    // When the flag is set, the page is in physical memory.
        uint32_t read_or_write      : 1;    // When the flag is set, the page can be read and written into.
        uint32_t user_or_supervisor : 1;    // When this flag is clear, the page is assigned the supervisor privilege level.
        uint32_t write_through      : 1;    // 1 indicates write-through caching is enabled, otherwise write-back caching is enabled
        uint32_t cache_disabled     : 1;    // 1 indicates the caching of individual pages or page tables is disabled
        uint32_t accessed           : 1;
        uint32_t dirty              : 1;
        uint32_t page_size          : 1;    // 1 indicates 4 MBytes
        uint32_t global_page        : 1;    // 1 indicates a global page
        uint32_t avail              : 3;    // available for system programmer’s use
        uint32_t pat                : 1;    // Page Table Attribute Index, for processors that do not support the PAT, this bit is reserved and should be set to 0.
        uint32_t reserved           : 9;    // must be set to 0
        uint32_t base_address       : 10;   // page base address
    } MByte __attribute__ ((packed));
} page_directory_entry_t;

typedef union page_table_entry {
    uint32_t val;
    struct {
        uint32_t present            : 1;    // When the flag is set, the page is in physical memory.
        uint32_t read_or_write      : 1;    // When the flag is set, the page can be read and written into.
        uint32_t user_or_supervisor : 1;    // When this flag is clear, the page is assigned the supervisor privilege level.
        uint32_t write_through      : 1;    // 1 indicates write-through caching is enabled, otherwise write-back caching is enabled
        uint32_t cache_disabled     : 1;    // 1 indicates the caching of individual pages or page tables is disabled
        uint32_t accessed           : 1;
        uint32_t dirty              : 1;
        uint32_t pat                : 1;    // Page Table Attribute Index
        uint32_t global_page        : 1;    // 1 indicates a global page
        uint32_t avail              : 3;    // available for system programmer’s use
        uint32_t base_address       : 20;   // page base address
    } KByte __attribute__ ((packed));
} page_table_entry_t;

// create kernel page directory and page table
page_directory_entry_t kernel_page_dir[1024] __attribute__((aligned(4096)));
page_table_entry_t kernel_page_table_0_4M[1024] __attribute__((aligned(4096)));

/** initialize page directory and page tabel in memory */
void paging_init_kernel(void);

/** set control registers (CR0, CR4) to enable paging */
extern void enable_paging(void);

/** load CR3 (PDBR) */
extern void load_CR3(uint32_t base);

/** print page dir content in screen */
void show_page_dir(page_directory_entry_t *page_dir);

/** print page table content in screen */
void show_page_table(page_table_entry_t *page_table);

#endif /* _PAGING_H */
