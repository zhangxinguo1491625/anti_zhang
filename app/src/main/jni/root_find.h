//
// Created by 22812 on 2025/9/11.
//

#ifndef ZHANG_ANTI_ROOT_FIND_H
#define ZHANG_ANTI_ROOT_FIND_H

#include <string>
#include <vector>
class CRootChecker {

public:
    CRootChecker() {}

    bool Check_Root();

private:
    bool IsRootProperty();
    bool IsRoot();
    bool IsSuFilesExist();
};

#endif //ZHANG_ANTI_ROOT_FIND_H