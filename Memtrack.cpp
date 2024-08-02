/*
 * Copyright (C) 2020 The Android Open Source Project
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

#include <vector>
#include <cstdio>
#include <string>
#include <sstream>
#include <fstream>

#include "Memtrack.h"

#include <android-base/logging.h>

namespace aidl {
namespace android {
namespace hardware {
namespace memtrack {

/*
 * The video memory is monitored through sysfs
 */
constexpr char kGraphicsMemoryFile[]{"/sys/devices/platform/soc@0/42080000.rifsc/48280000.gpu/vidmem"};

ndk::ScopedAStatus Memtrack::getMemory(int pid, MemtrackType type,
                                       std::vector<MemtrackRecord>* _aidl_return) {
    if (pid < 0) {
        return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_ILLEGAL_ARGUMENT));
    }
    if (type != MemtrackType::OTHER && type != MemtrackType::GL && type != MemtrackType::GRAPHICS &&
        type != MemtrackType::MULTIMEDIA && type != MemtrackType::CAMERA) {
        return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
    }

    if (type == MemtrackType::GRAPHICS) {
        return getGraphicsMemory(pid, _aidl_return);
    }

    _aidl_return->clear();
    return ndk::ScopedAStatus::ok();
}

/**
 * Get back dedicated GPU memory
 * 
 * @param pid: process for which memory information is requested
 * @param vector of MemtrackRecord containing memory information
 * @return status
 */
ndk::ScopedAStatus Memtrack::getGraphicsMemory(int pid,
                                 std::vector<MemtrackRecord>* _aidl_return) {

    long size = 0;
    volatile bool is_pidfound = false;

    std::ifstream stream(kGraphicsMemoryFile);
    if (!stream.is_open()) {
        PLOG(ERROR) << "failed to open file" << kGraphicsMemoryFile;
        _aidl_return->clear();
        return ndk::ScopedAStatus::ok();
    }

    /* Format:
     * VidMem Usage (Process 126: surfaceflinger):
     *              Current     Maximum     Total
     * All-Types    16361320    16397908    17045756
     */
    std::string spid = std::to_string(pid) + ":";

    std::string line;
    while (std::getline(stream, line)) {
        if (! is_pidfound) {
            size_t found = line.find("VidMem");
            if ((found !=  std::string::npos) && (found == 0)) {
                found = line.find(spid);
                if ((found !=  std::string::npos) && (found == 3)) {
                    is_pidfound = true;
                }
            }
        } else {
            size_t found = line.find("All-Types");
            if (found !=  std::string::npos) {
                int ret = std::sscanf(line.c_str(), "%*s %lu %*lu %*lu\n", &size);
                if (ret != 1) {
                    PLOG(ERROR) << "failed to extract size from line:" << line.c_str();
                    continue;
                }
                break;
            }
        }
    }

    if (size > 0) {
        MemtrackRecord record;
        record.sizeInBytes = size;
        record.flags = MemtrackRecord::FLAG_SMAPS_UNACCOUNTED |
                 MemtrackRecord::FLAG_PRIVATE |
                 MemtrackRecord::FLAG_DEDICATED |
                 MemtrackRecord::FLAG_NONSECURE;
        _aidl_return->emplace_back(record);
    } else {
        _aidl_return->clear();
    }
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Memtrack::getGpuDeviceInfo(std::vector<DeviceInfo>* _aidl_return) {
    _aidl_return->clear();
    DeviceInfo dev_info = {.id = 0, .name = "gcnano"};
    _aidl_return->emplace_back(dev_info);
    return ndk::ScopedAStatus::ok();
}

}  // namespace memtrack
}  // namespace hardware
}  // namespace android
}  // namespace aidl
