//
// Created by 22812 on 2025/10/15.
//

#include "Common_Sensor.h"
#include <vector>
#include <string>
#include <android/sensor.h>
#include <android/log.h>

namespace CCommon_Sensor {
    std::string get_sensor(){
        std::string sensor_list = "[";
        ASensorManager* sensorManager = ASensorManager_getInstance();
        if (!sensorManager) {
            return sensor_list;
        }

        // 获取传感器列表
        ASensorList sensorList;
        int sensorCount = ASensorManager_getSensorList(sensorManager, &sensorList);
        std::string type_str;
        // 遍历所有传感器
        for (int i = 0; i < sensorCount; i++) {
            const ASensor* sensor = sensorList[i];
            const char* name = ASensor_getName(sensor);
            const char* vendor = ASensor_getVendor(sensor);
            int type = ASensor_getType(sensor);
            float resolution = ASensor_getResolution(sensor);
            __android_log_print(ANDROID_LOG_DEBUG, "get_sensor", "[%d] name=%s, vendor=%s, type=%d, resolution=%f",
            i, name, vendor, type, resolution);
            type_str = std::to_string(type);
            sensor_list.append(type_str + ",");
        }
        sensor_list[sensor_list.size() - 1] = ']';
        return sensor_list;
    }
} // CCommon_Sensor