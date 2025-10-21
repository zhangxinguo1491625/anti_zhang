//
// Created by 22812 on 2025/10/20.
//

#ifndef ZHANG_ANTI_UNICORN_CHECK_H
#define ZHANG_ANTI_UNICORN_CHECK_H

namespace Unicorn_Check {
    bool Check_Unicorn(JNIEnv* env);
    bool checkUnicornByUname();
    bool checkUnicornByStaticMethodId(JNIEnv* env);
    bool checkUnicornByStaticMethodHash(JNIEnv* env);
    int java_hashCode(const char *str);

} // Unicorn_Check

#endif //ZHANG_ANTI_UNICORN_CHECK_H
