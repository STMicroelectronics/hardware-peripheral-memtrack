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

#ifndef _MEMTRACK_STM_H_
#define _MEMTRACK_STM_H_

/*
 * By default the video memory is monitored through debugfs if available
 */
#ifdef DEBUGFS
const char *kGraphicsMemory = "/d/gc/vidmem";
#else
const char *kGraphicsMemory = "/sys/devices/platform/soc/59000000.gpu/vidmem";
#endif

int memtrack_get_graphics_memory(pid_t pid,
                        struct memtrack_record *records,
                        size_t *num_records);

#endif
