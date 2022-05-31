/**
 * @file paging.c
 * @brief Definitions of paging-related functions
 * @version 0.1
 * @date 2022-03-17
 */

#include "paging.h"

#define VIDEO                  0xB8000
#define VIDEO_page_table_index (VIDEO >> 12)

/**
 * brief: initialize kernel page directory and page tabel in memory
 * input: none
 * output: set entries in page directory and page table
 * return: none
 * side effects: none
 */
void paging_init_kernel(void) {
    int i;
    // defines page directory entry for video memory
    // it shoud be of index 0 in page directory in order to map 0xB8000 virtual to 0xB8000 physical
    {
        page_directory_entry_t pde_4K_0_4M;
        pde_4K_0_4M.KByte.present               = 0x1;
        pde_4K_0_4M.KByte.read_or_write         = 0x1;
        pde_4K_0_4M.KByte.user_or_supervisor    = 0x0;
        pde_4K_0_4M.KByte.write_through         = 0x0;
        pde_4K_0_4M.KByte.cache_disabled        = 0x0;
        pde_4K_0_4M.KByte.accessed              = 0x0;
        pde_4K_0_4M.KByte.reserved              = 0x0;
        pde_4K_0_4M.KByte.page_size             = 0x0;
        pde_4K_0_4M.KByte.global_page           = 0x0;
        pde_4K_0_4M.KByte.avail                 = 0x0;
        pde_4K_0_4M.KByte.base_address          = ((uint32_t)kernel_page_table_0_4M) >> 12;  // base address of kernel_page_table_0_4M, use right shift to get the highest 20 bits

        kernel_page_dir[0] = pde_4K_0_4M;
    }

    // defines page directory entry for kernel
    // it shoud be of index 1 in page directory in order to map 4MB virtual to 4MB physical
    {
        page_directory_entry_t pde_4M_4_8M;
        pde_4M_4_8M.MByte.present                 = 0x1;
        pde_4M_4_8M.MByte.read_or_write           = 0x1;
        pde_4M_4_8M.MByte.user_or_supervisor      = 0x0;
        pde_4M_4_8M.MByte.write_through           = 0x0;
        pde_4M_4_8M.MByte.cache_disabled          = 0x1;
        pde_4M_4_8M.MByte.accessed                = 0x0;
        pde_4M_4_8M.MByte.dirty                   = 0x0;
        pde_4M_4_8M.MByte.page_size               = 0x1;
        pde_4M_4_8M.MByte.global_page             = 0x1;
        pde_4M_4_8M.MByte.avail                   = 0x0;
        pde_4M_4_8M.MByte.pat                     = 0x0;
        pde_4M_4_8M.MByte.reserved                = 0x0;
        pde_4M_4_8M.MByte.base_address            = 0x1;  // map 4MB virtual to 4MB physical

        kernel_page_dir[1] = pde_4M_4_8M;
    }

    // set the remaing entry as not present
    {
        page_directory_entry_t not_present_pde;
        not_present_pde.val = 0x0;
        for (i = 2; i < 1024; i++) {
            kernel_page_dir[i] = not_present_pde;
        }
    }

    // initialize the kernel_page_table_0_4M
    {
        page_table_entry_t not_present_pte;
        not_present_pte.val = 0x0;
        page_table_entry_t video_memory_pte;
        video_memory_pte.KByte.present                = 0x1;
        video_memory_pte.KByte.read_or_write          = 0x1;
        video_memory_pte.KByte.user_or_supervisor     = 0x0;
        video_memory_pte.KByte.write_through          = 0x0;
        video_memory_pte.KByte.cache_disabled         = 0x0;
        video_memory_pte.KByte.accessed               = 0x0;
        video_memory_pte.KByte.dirty                  = 0x0;
        video_memory_pte.KByte.pat                    = 0x0;
        video_memory_pte.KByte.global_page            = 0x0;
        video_memory_pte.KByte.avail                  = 0x0;
        video_memory_pte.KByte.base_address           = VIDEO >> 12; // use right shift to get the highest 20 bits
        for (i = 0; i < 1024; i++) {
            if (VIDEO_page_table_index == i) {
                kernel_page_table_0_4M[i] = video_memory_pte;
            } else {
                kernel_page_table_0_4M[i] = not_present_pte;
            }
        }
    }
}

/**
 * brief: print page dir content into screen
 * input: ptr to page_dir
 * output: none
 * side effect: change video memory
 */
void show_page_dir(page_directory_entry_t *page_dir)
{
    uint32_t i, meet_not_present;
    printf("page dir at %#x\n", page_dir);
    printf("|index    P    base    G    PS    U/S    R/W\n");
    for (i = 0, meet_not_present = 0; i < 1024; i++)
    {
        if (0 == page_dir[i].KByte.present || 0 == page_dir[i].MByte.present)
        {
            if (0 == meet_not_present)
            {
                printf("|...    0\n");
                meet_not_present = 1;
            }
        }
        else
        {
            meet_not_present = 0;
            if (1 == page_dir[i].MByte.page_size)
            {
                printf("|%u    1    %#x    %u    4M    %u    %u\n", i, page_dir[i].MByte.base_address, page_dir[i].MByte.global_page, page_dir[i].MByte.write_through, page_dir[i].MByte.read_or_write);
            }
            else
            {
                printf("|%u    1    %#x    %u    4K    %u    %u\n", i, page_dir[i].KByte.base_address, page_dir[i].KByte.global_page, page_dir[i].KByte.write_through, page_dir[i].KByte.read_or_write);
            }
        }
    }
    printf("\n");
}

/**
 * brief: print page table content into screen
 * input: ptr to page_table
 * output: none
 * side effect: change video memory
 */
void show_page_table(page_table_entry_t *page_table)
{
    uint32_t i, meet_not_present;
    printf("page table at %#x\n", page_table);
    printf("|index    P    base    G    U/S    R/W\n");
    for (i = 0, meet_not_present = 0; i < 1024; i++)
    {
        if (0 == page_table[i].KByte.present)
        {
            if (0 == meet_not_present)
            {
                printf("|...    0\n");
                meet_not_present = 1;
            }
        }
        else
        {
            meet_not_present = 0;
                printf("|%u    1    %#x    %u    %u    %u\n", i, page_table[i].KByte.base_address, page_table[i].KByte.global_page, page_table[i].KByte.write_through, page_table[i].KByte.read_or_write);
        }
    }
    printf("\n");
}
