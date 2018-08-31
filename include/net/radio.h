/*
 * @copyright 2018 blik GmbH
 *
 * @author Franco Saworski <franco.saworski@blik.io>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Public Radio API
 */
#pragma once

#include <device.h>
#include <logging/sys_log.h>
#include <net/buf.h>
#include <net/radio_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * @brief Get bitmask of supported radio features
 *
 * @param dev Pointer to the device structure for the driver instance
 *
 * @return Bitmask of supported radio features
 */
static inline enum ieee802154_hw_caps radio_get_capabilities(
		struct device *dev)
{
	const struct radio_api *api = dev->driver_api;

	if (api == NULL) {
		SYS_LOG_ERR("Could not bind to device API.");
		return 0;
	}

	return api->get_capabilities(dev);
}

/*
 * @brief Transmit a data fragment
 *
 * @param dev Pointer to the device structure for the driver instance
 * @param frag Pointer to net_buf_simple data struct of the data to be sent
 *
 * @return ERRNO type values
 * @retval 0 on success
 * @retval -EFAULT Driver API could not be bound
 * @retval <0 otherwise
 */
static inline int radio_send(struct device *dev,
		struct net_buf_simple *frag)
{
	const struct radio_api *api = dev->driver_api;

	if (api == NULL) {
		SYS_LOG_ERR("Could not bind to device API.");
		return -EFAULT;
	}

	return api->tx(dev, frag);
}

/*
 * @brief Query the radio driver's RX queue for one new packet
 *
 * @param dev Pointer to the device structure for the driver instance
 * @param timeout Timeout value to wait for a packet to arrive in the RX queue
 *
 * @return ERRNO type values
 * @retval 0 on success
 * @retval -EFAULT Driver API could not be bound
 * @retval <0 otherwise
 */
static inline int radio_recv(struct device *dev, u8_t *data,
		u8_t *data_len, u32_t timeout_ms)
{
	const struct radio_api *api = dev->driver_api;

	if (api == NULL) {
		SYS_LOG_ERR("Could not bind to device API.");
		return -EFAULT;
	}

	/* k_fifo_get returns pointer to a net_pkt object */
	return api->rx(dev, data, data_len, timeout_ms);
}

/*
 * @brief Check the radio channels activity with CCA
 *
 * @param dev Pointer to the device structure for the driver instance
 *
 * @return ERRNO type values
 * @retval 0 on success
 * @retval -EFAULT Driver API could not be bound
 * @retval <0 otherwise
 */
static inline int radio_cca(struct device *dev)
{
	const struct radio_api *api = dev->driver_api;

	if (api == NULL) {
		SYS_LOG_ERR("Could not bind to device API.");
		return -EFAULT;
	}

	return api->cca(dev);
}

/*
 * @brief Set radio channel
 *
 * @param dev Pointer to the device structure for the driver instance
 * @param channel Valid channel number for the radio
 *
 * @return ERRNO type values
 * @retval 0 on success
 * @retval -EFAULT Driver API could not be bound
 * @retval <0 otherwise
 */
static inline int radio_set_channel(struct device *dev, u16_t channel)
{
	const struct radio_api *api = dev->driver_api;

	if (api == NULL) {
		SYS_LOG_ERR("Could not bind to device API.");
		return -EFAULT;
	}

	return api->set_channel(dev, channel);
}

/*
 * @brief Set the address filters (for IEEE802154_HW_FILTER)
 *
 * @param dev Pointer to the device structure for the driver instance
 *
 * @return ERRNO type values
 * @retval 0 on success
 * @retval -EFAULT Driver API could not be bound
 * @retval <0 otherwise
 */
static inline int radio_set_filter(struct device *dev,
		  enum ieee802154_filter_type type,
		  const struct ieee802154_filter *filter)
{
	const struct radio_api *api = dev->driver_api;

	if (api == NULL) {
		SYS_LOG_ERR("Could not bind to device API.");
		return -EFAULT;
	}

	return api->set_filter(dev, type, filter);
}

/*
 * @brief Set the TX power level in dbm
 *
 * @param dev Pointer to the device structure for the driver instance
 * @param dbm Valid power level for the radio
 *
 * @return ERRNO type values
 * @retval 0 on success
 * @retval -EFAULT Driver API could not be bound
 * @retval <0 otherwise
 */
static inline int radio_set_txpower(struct device *dev, s16_t dbm)
{
	const struct radio_api *api = dev->driver_api;

	if (api == NULL) {
		SYS_LOG_ERR("Could not bind to device API.");
		return -EFAULT;
	}

	return api->set_txpower(dev, dbm);
}

/*
 * @brief Start the radio
 *
 * @param dev Pointer to the device structure for the driver instance
 *
 * @return ERRNO type values
 * @retval 0 on success
 * @retval -EFAULT Driver API could not be bound
 * @retval <0 otherwise
 */
static inline int radio_start(struct device *dev)
{
	const struct radio_api *api = dev->driver_api;

	if (api == NULL) {
		SYS_LOG_ERR("Could not bind to device API.");
		return -EFAULT;
	}

	return api->start(dev);
}

/*
 * @brief Stop the radio
 *
 * @param dev Pointer to the device structure for the driver instance
 *
 * @return ERRNO type values
 * @retval 0 on success
 * @retval -EFAULT Driver API could not be bound
 * @retval <0 otherwise
 */
static inline int radio_stop(struct device *dev)
{
	const struct radio_api *api = dev->driver_api;

	if (api == NULL) {
		SYS_LOG_ERR("Could not bind to device API.");
		return -EFAULT;
	}

	return api->stop(dev);
}

#ifdef __cplusplus
}
#endif
