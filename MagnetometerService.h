/* mbed Microcontroller Library
 * Copyright (c) 2006-2020 ARM Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* MBED_DEPRECATED */
#include <cmath>
#include <cstdint>
#warning "These services are deprecated and will be removed. Please see services.md for details about replacement services."

#ifndef MBED_BLE_MAGNETO_SERVICE_H__
#define MBED_BLE_MAGNETO_SERVICE_H__

#include "ble/BLE.h"
#include "ble/Gap.h"
#include "ble/GattServer.h"

#if BLE_FEATURE_GATT_SERVER

/**
 * BLE Heart Rate Service.
 *
 * @par purpose
 *
 * Fitness applications use the heart rate service to expose the heart
 * beat per minute measured by a heart rate sensor.
 *
 * Clients can read the intended location of the sensor and the last heart rate
 * value measured. Additionally, clients can subscribe to server initiated
 * updates of the heart rate value measured by the sensor. The service delivers
 * these updates to the subscribed client in a notification packet.
 *
 * The subscription mechanism is useful to save power; it avoids unecessary data
 * traffic between the client and the server, which may be induced by polling the
 * value of the heart rate measurement characteristic.
 *
 * @par usage
 *
 * When this class is instantiated, it adds a heart rate service in the GattServer.
 * The service contains the location of the sensor and the initial value measured
 * by the sensor.
 *
 * Application code can invoke updateHeartRate() when a new heart rate measurement
 * is acquired; this function updates the value of the heart rate measurement
 * characteristic and notifies the new value to subscribed clients.
 *
 * @note You can find specification of the heart rate service here:
 * https://www.bluetooth.com/specifications/gatt
 *
 * @attention The service does not expose information related to the sensor
 * contact, the accumulated energy expanded or the interbeat intervals.
 *
 * @attention The heart rate profile limits the number of instantiations of the
 * heart rate services to one.
 */
class MagnetoService {

public:
    /**
     * Construct and initialize a heart rate service.
     *
     * The construction process adds a GATT heart rate service in @p _ble
     * GattServer, sets the value of the heart rate measurement characteristic
     * to @p hrmCounter and the value of the body sensor location characteristic
     * to @p location.
     *
     * @param[in] _ble BLE device that hosts the heart rate service.
     * @param[in] hrmCounter Heart beats per minute measured by the heart rate
     * sensor.
     * @param[in] location Intended location of the heart rate sensor.
     */
    MagnetoService(BLE &_ble, int16_t magnetoX, int16_t magnetoY, int16_t magnetoZ):
        ble(_ble),
        valueBytes(magnetoX, magnetoY, magnetoZ),
        magnetoXYZ(
            GattCharacteristic::UUID_MAGNETO_MEASUREMENT_CHAR,
            (uint8_t*)valueBytes.getPointer(),
            valueBytes.getNumValueBytes(),
            valueBytes.getNumValueBytes(),
            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY
        )
    {
        setupService();
    }

    /**
     * Update the heart rate that the service exposes.
     *
     * The server sends a notification of the new value to clients that have
     * subscribed to updates of the heart rate measurement characteristic; clients
     * reading the heart rate measurement characteristic after the update obtain
     * the updated value.
     *
     * @param[in] hrmCounter Heart rate measured in BPM.
     *
     * @attention This function must be called in the execution context of the
     * BLE stack.
     */
    void updateMagneto(int16_t magnetoX, int16_t magnetoY, int16_t magnetoZ) {
        valueBytes.updateHeartRate(magnetoX, magnetoY, magnetoZ);
        ble.gattServer().write(
            magnetoXYZ.getValueHandle(),
            (uint8_t*)valueBytes.getPointer(),
            valueBytes.getNumValueBytes()
        );
    }

protected:
    /**
     * Construct and add to the GattServer the heart rate service.
     */
    void setupService() {
        GattCharacteristic *charTable[] = {
            &magnetoXYZ,
        };
        GattService magService(
            GattService::UUID_MAGNETO_SERVICE,
            charTable,
            sizeof(charTable) / sizeof(charTable[0])
        );

        ble.gattServer().addService(magService);
    }

protected:
    /*
     * Magneto measurement value.
     */
    struct MagnetoValueBytes {
        /* 1 byte for the Flags, and up to two bytes for heart rate value. */
        static const unsigned MAGNETO_NUM = 3;
        static const unsigned MAGNETO_X_IDX = 0;
        static const unsigned MAGNETO_Y_IDX = 1;
        static const unsigned MAGNETO_Z_IDX = 2;

        MagnetoValueBytes(int16_t magnetoX, int16_t magnetoY, int16_t magnetoZ) : valueBytes()
        {
            updateHeartRate(magnetoX, magnetoY, magnetoZ);
        }

        void updateHeartRate(int16_t magnetoX, int16_t magnetoY, int16_t magnetoZ)
        {
            valueBytes[MAGNETO_X_IDX] = magnetoX;
            valueBytes[MAGNETO_Y_IDX] = magnetoY;
            valueBytes[MAGNETO_Z_IDX] = magnetoZ;
        }

        int16_t *getPointer()
        {
            return valueBytes;
        }

        const int16_t *getPointer() const
        {
            return valueBytes;
        }

        unsigned getNumValueBytes() const
        {
            return 1 + sizeof(int16_t)*MAGNETO_NUM;
        }

    private:
        int16_t valueBytes[MAGNETO_NUM];
    };

protected:
    BLE &ble;
    MagnetoValueBytes valueBytes;
    GattCharacteristic magnetoXYZ;
};

#endif // BLE_FEATURE_GATT_SERVER

#endif /* #ifndef MBED_BLE_HEART_RATE_SERVICE_H__*/
