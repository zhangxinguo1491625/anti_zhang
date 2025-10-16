//
// Created by 22812 on 2025/9/16.
//

#ifndef ZHANG_ANTI_EMULATORCHECKER_H
#define ZHANG_ANTI_EMULATORCHECKER_H

namespace CEmulatorChecker {
    bool Check_Emulator();
    bool isEmulator2();
    bool isEmulator1();
    bool IsVirtualBoxBased();
    bool GetLinkerAbi();
    bool isArm64();
    bool libcisarm64();

} // CEmulatorChecker

#endif //ZHANG_ANTI_EMULATORCHECKER_H
