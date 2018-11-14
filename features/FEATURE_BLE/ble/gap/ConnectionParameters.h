/* mbed Microcontroller Library
 * Copyright (c) 2018 ARM Limited
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

#ifndef MBED_EXTENDED_CONNECT_PARAMETERS_H__
#define MBED_EXTENDED_CONNECT_PARAMETERS_H__

#include "ble/BLETypes.h"
#include "mbed_assert.h"

/**
 * @addtogroup ble
 * @{
 * @addtogroup gap
 * @{
 */

namespace ble {

class ConnectionParameters {
    static const uint8_t MAX_PARAM_PHYS = 3;
public:
    ConnectionParameters() :
        _filterPolicy(ble::SCAN_POLICY_IGNORE_WHITELIST),
        _ownAddressType(ble::own_address_type_t::PUBLIC)
    {
        for (uint8_t i = 0; i < MAX_PARAM_PHYS; ++i) {
            _scanInterval[i] = 4;
            _scanWindow[i] = 4;
            _minConnectionInterval[i] = 6;
            _maxConnectionInterval[i] = 0xC80;
            _slaveLatency[i] = 0;
            _connectionSupervisionTimeout[i] = 0xC80;
            _minEventLength[i] = 0;
            _maxEventLength[i] = 0xFFFF;
            _enabledPhy[i] = false;
        }
    };

    /* setters */

    ConnectionParameters &setScanParameters(
        ble::scan_interval_t scanInterval,
        ble::scan_window_t scanWindow,
        ble::phy_t phy = ble::phy_t::LE_1M
    )
    {
        uint8_t phy_index = handlePhyToggle(phy, true);

        _scanInterval[phy_index] = scanInterval.value();
        _scanWindow[phy_index] = scanWindow.value();

        return *this;
    }

    ConnectionParameters &setConnectionParameters(
        ble::conn_interval_t minConnectionInterval,
        ble::conn_interval_t maxConnectionInterval,
        ble::slave_latency_t slaveLatency,
        ble::supervision_timeout_t connectionSupervisionTimeout,
        ble::phy_t phy = ble::phy_t::LE_1M,
        ble::conn_event_length_t minEventLength = ble::conn_event_length_t(0),
        ble::conn_event_length_t maxEventLength = ble::conn_event_length_t(
            0xFFFF
        )
    )
    {
        uint8_t phy_index = handlePhyToggle(phy, true);

        _minConnectionInterval[phy_index] = minConnectionInterval.value();
        _maxConnectionInterval[phy_index] = maxConnectionInterval.value();
        _slaveLatency[phy_index] = slaveLatency.value();
        _connectionSupervisionTimeout[phy_index] = connectionSupervisionTimeout.value();

        /* avoid overflows and truncation */
        if (minEventLength.value() > maxEventLength.value()) {
            minEventLength = maxEventLength;
        }

        _minEventLength[phy_index] = minEventLength.value();
        _maxEventLength[phy_index] = maxEventLength.value();

        return *this;
    }

    ConnectionParameters &setOwnAddressType(
        ble::own_address_type_t ownAddress
    )
    {
        _ownAddressType = ownAddress;

        return *this;
    }

    ConnectionParameters &setFilterPolicy(
        ble::scanning_policy_mode_t filterPolicy
    )
    {
        _filterPolicy = filterPolicy;

        return *this;
    }

    ConnectionParameters &togglePhy(
        bool phy1M,
        bool phy2M,
        bool phyCoded
    )
    {
        handlePhyToggle(ble::phy_t::LE_1M, phy1M);
        handlePhyToggle(ble::phy_t::LE_2M, phy2M);
        handlePhyToggle(ble::phy_t::LE_CODED, phyCoded);

        return *this;
    }

    ConnectionParameters &disablePhy(
        ble::phy_t phy = ble::phy_t::LE_1M
    )
    {
        handlePhyToggle(phy, false);

        return *this;
    }

    ConnectionParameters &enablePhy(
        ble::phy_t phy = ble::phy_t::LE_1M
    )
    {
        handlePhyToggle(phy, true);

        return *this;
    }

    /* getters */

    ble::own_address_type_t getOwnAddressType() const
    {
        return _ownAddressType;
    }

    ble::scanning_policy_mode_t getFilterPolicy() const
    {
        return _filterPolicy;
    }

    uint8_t getNumberOfEnabledPhys() const
    {
        return (
            _enabledPhy[ble::phy_t::LE_1M] * 1 +
                _enabledPhy[ble::phy_t::LE_2M] * 1 +
                _enabledPhy[ble::phy_t::LE_CODED] * 1
        );
    }

    uint8_t getPhySet() const
    {
        ble::phy_set_t set(
            _enabledPhy[ble::phy_t::LE_1M],
            _enabledPhy[ble::phy_t::LE_2M],
            _enabledPhy[ble::phy_t::LE_CODED]
        );
        return set.value();
    }

    /* these return pointers to arrays of settings valid only across the number of active PHYs */

    const uint16_t *getScanIntervalArray() const
    {
        return &_scanInterval[getFirstEnabledIndex()];
    }

    const uint16_t *getScanWindowArray() const
    {
        return &_scanWindow[getFirstEnabledIndex()];
    }

    const uint16_t *getMinConnectionIntervalArray() const
    {
        return &_minConnectionInterval[getFirstEnabledIndex()];
    }

    const uint16_t *getMaxConnectionIntervalArray() const
    {
        return &_maxConnectionInterval[getFirstEnabledIndex()];
    }

    const uint16_t *getSlaveLatencyArray() const
    {
        return &_slaveLatency[getFirstEnabledIndex()];
    }

    const uint16_t *getConnectionSupervisionTimeoutArray() const
    {
        return &_connectionSupervisionTimeout[getFirstEnabledIndex()];
    }

    const uint16_t *getMinEventLengthArray() const
    {
        return &_minEventLength[getFirstEnabledIndex()];
    }

    const uint16_t *getMaxEventLengthArray() const
    {
        return &_maxEventLength[getFirstEnabledIndex()];
    }

private:
    uint8_t getFirstEnabledIndex() const
    {
        if (_enabledPhy[ble::phy_t::LE_1M]) {
            return 0;
        } else if (_enabledPhy[ble::phy_t::LE_2M]) {
            return 1;
        } else if (_enabledPhy[ble::phy_t::LE_CODED]) {
            return 2;
        }
        /* this should never happen, it means you were trying to start a connection with a blank set
         * of paramters - you need to enabled at least one phy */
        MBED_ASSERT(
            "Trying to use connection parameters without any PHY defined.");
        return 0;
    }

    /** Handle toggling PHYs on and off and return the correct index to use to set the configuration elements.
     *
     * @param phy Which Phy is being toggle.
     * @param enable On or Off.
     * @return The index to the array of settings.
     */
    uint8_t handlePhyToggle(ble::phy_t phy, bool enable)
    {
        uint8_t index = phy.value();

        bool was_swapped = false;
        bool is_swapped = false;

        if (isSwapped()) {
            was_swapped = true;
        }

        _enabledPhy[phy.value()] = enable;

        if (isSwapped()) {
            is_swapped = true;
        }

        if (was_swapped != is_swapped) {
            swapCodedAnd2M();
        }

        if (is_swapped && phy == ble::phy_t::LE_CODED) {
            index -= 1;
        }

        return index;
    }

    bool isSwapped() const
    {
        return (
            _enabledPhy[ble::phy_t::LE_1M] &&
                !_enabledPhy[ble::phy_t::LE_2M] &&
                _enabledPhy[ble::phy_t::LE_CODED]
        );
    }

    /** Handle the swapping of 2M and CODED so that the array is ready for the pal call. */
    void swapCodedAnd2M()
    {
        uint16_t scanInterval = _scanInterval[ble::phy_t::LE_2M];
        uint16_t scanWindow = _scanWindow[ble::phy_t::LE_2M];
        uint16_t minConnectionInterval = _minConnectionInterval[ble::phy_t::LE_2M];
        uint16_t maxConnectionInterval = _maxConnectionInterval[ble::phy_t::LE_2M];
        uint16_t slaveLatency = _maxConnectionInterval[ble::phy_t::LE_2M];
        uint16_t connectionSupervisionTimeout = _connectionSupervisionTimeout[ble::phy_t::LE_2M];
        uint16_t minEventLength = _minEventLength[ble::phy_t::LE_2M];
        uint16_t maxEventLength = _maxEventLength[ble::phy_t::LE_2M];

        _scanInterval[ble::phy_t::LE_2M] = _scanInterval[ble::phy_t::LE_CODED];
        _scanWindow[ble::phy_t::LE_2M] = _scanWindow[ble::phy_t::LE_CODED];
        _minConnectionInterval[ble::phy_t::LE_2M] = _minConnectionInterval[ble::phy_t::LE_CODED];
        _maxConnectionInterval[ble::phy_t::LE_2M] = _maxConnectionInterval[ble::phy_t::LE_CODED];
        _slaveLatency[ble::phy_t::LE_2M] = _slaveLatency[ble::phy_t::LE_CODED];
        _connectionSupervisionTimeout[ble::phy_t::LE_2M] = _connectionSupervisionTimeout[ble::phy_t::LE_CODED];
        _minEventLength[ble::phy_t::LE_2M] = _minEventLength[ble::phy_t::LE_CODED];
        _maxEventLength[ble::phy_t::LE_2M] = _maxEventLength[ble::phy_t::LE_CODED];

        _scanInterval[ble::phy_t::LE_CODED] = scanInterval;
        _scanWindow[ble::phy_t::LE_CODED] = scanWindow;
        _minConnectionInterval[ble::phy_t::LE_CODED] = minConnectionInterval;
        _maxConnectionInterval[ble::phy_t::LE_CODED] = maxConnectionInterval;
        _slaveLatency[ble::phy_t::LE_CODED] = slaveLatency;
        _connectionSupervisionTimeout[ble::phy_t::LE_CODED] = connectionSupervisionTimeout;
        _minEventLength[ble::phy_t::LE_CODED] = minEventLength;
        _maxEventLength[ble::phy_t::LE_CODED] = maxEventLength;
    }

private:
    ble::scanning_policy_mode_t _filterPolicy;
    ble::own_address_type_t _ownAddressType;

    uint16_t _scanInterval[MAX_PARAM_PHYS]; /* 0.625 ms */
    uint16_t _scanWindow[MAX_PARAM_PHYS]; /* 0.625 ms */
    uint16_t _minConnectionInterval[MAX_PARAM_PHYS]; /* 1.25 ms */
    uint16_t _maxConnectionInterval[MAX_PARAM_PHYS]; /* 1.25 ms */
    uint16_t _slaveLatency[MAX_PARAM_PHYS]; /* events */
    uint16_t _connectionSupervisionTimeout[MAX_PARAM_PHYS]; /* 10 ms */
    uint16_t _minEventLength[MAX_PARAM_PHYS]; /* 0.625 ms */
    uint16_t _maxEventLength[MAX_PARAM_PHYS]; /* 0.625 ms */

    bool _enabledPhy[MAX_PARAM_PHYS];
};

} // namespace ble

/**
 * @}
 * @}
 */

#endif /* ifndef MBED_EXTENDED_CONNECT_PARAMETERS_H__ */
