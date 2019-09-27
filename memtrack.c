/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOG_TAG "STM memtrack"
#include <log/log.h>

#include <hardware/memtrack.h>

#include "memtrack.h"

int memtrack_init(const struct memtrack_module *module)
{
    if (!module)
        return -EPERM;

    return 0;
}

int memtrack_get_memory(const struct memtrack_module *module,
                        pid_t pid,
                        int type,
                        struct memtrack_record *records,
                        size_t *num_records)
{
    if (!module)
        return -EPERM;

    if (type == MEMTRACK_TYPE_GRAPHICS) {
        return memtrack_get_graphics_memory(pid, records, num_records);
    }

    return -ENODEV;
}

int memtrack_get_graphics_memory(pid_t pid,
                        struct memtrack_record *records,
                        size_t *num_records)
{
    size_t allocated_records = *num_records;
    FILE *fp;
    char line[1024];
    char scan_tag[8];
    int scan_pid = 0;
    unsigned long size = 0;
    int ret;

    volatile bool is_pidfound = false;

    *num_records = 1;

    // only unaccounted memory area
    if ((allocated_records == 0) || records == NULL) {
        return 0;
    }

    fp = fopen(kGraphicsMemory, "r");
    if (fp == NULL) {
        ALOGD("%s: failed to open file %s (%d)", __FUNCTION__, kGraphicsMemory, errno);
        return -errno;
    }

    while (1) {
        if (fgets(line, sizeof(line), fp) == NULL) {
           break;
        }

        if (! is_pidfound) {
            /* Format:
             * VidMem Usage (Process 126: surfaceflinger):
             */
            ret = sscanf(line, "%6s %*s %*s %d: %*s\n",
                    scan_tag, &scan_pid);

            if (ret != 2) {
               continue;
            }

            if (strcmp(scan_tag, "VidMem") == 0) {
//                ALOGD("%s: required pid = %d vs scanned pid = %d", __FUNCTION__, pid, scan_pid);
                if (pid == scan_pid) {
                    is_pidfound = true;
                }
            }
        } else {
            /* Format:
             *              Current     Maximum     Total
             * All-Types    16361320    16397908    17045756
             */
            ret = sscanf(line, "%*s %lu %*lu %*lu\n", &size);
            if (ret != 1) {
                continue;
            }
            ALOGD("%s: graphics memory size (pid = %d): %lu bytes", __FUNCTION__, pid, size);
            break;
        }
    }

    if (size > 0) {
        records[0].size_in_bytes = size;
        records[0].flags = MEMTRACK_FLAG_SMAPS_UNACCOUNTED |
                 MEMTRACK_FLAG_PRIVATE |
                 MEMTRACK_FLAG_DEDICATED |
                 MEMTRACK_FLAG_NONSECURE;
    }

    fclose(fp);

    return 0;
}

static int memtrack_open(__attribute__((unused)) const hw_module_t* module,
                    const char* name, hw_device_t** device)
{
    int ret = 0;

    if (strcmp(name, MEMTRACK_HARDWARE_MODULE_ID) != 0)
        return -EINVAL;

    struct memtrack_module *dev = (struct memtrack_module *)calloc(1,
            sizeof(struct memtrack_module));
    if (!dev)
        return -ENOMEM;

    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.module_api_version = MEMTRACK_MODULE_API_VERSION_0_1;
    dev->common.hal_api_version = HARDWARE_HAL_API_VERSION;

    dev->init = memtrack_init;
    dev->getMemory = memtrack_get_memory;

    *device = (hw_device_t*)dev;

    return ret;
}

static struct hw_module_methods_t memtrack_module_methods = {
    .open = memtrack_open,
};

struct memtrack_module HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = MEMTRACK_MODULE_API_VERSION_0_1,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = MEMTRACK_HARDWARE_MODULE_ID,
        .name = "Memory Tracker HAL",
        .author = "The Android Open Source Project",
        .methods = &memtrack_module_methods,
    },

    .init = memtrack_init,
    .getMemory = memtrack_get_memory,
};
