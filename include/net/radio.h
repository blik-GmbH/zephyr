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

/** Get the device capabilities */
__syscall enum ieee802154_hw_caps radio_get_capabilities(
		struct device *dev);

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

/** Transmit a packet fragment */
__syscall int radio_send(struct device *dev, struct net_buf_simple *frag);

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

/** A fifo for the driver to write RX packages into */
__syscall struct net_buf_simple *radio_recv(struct device *dev,
		u32_t timeout);

static inline struct net_buf_simple *radio_recv(struct device *dev,
		u32_t timeout)
{
	const struct radio_api *api = dev->driver_api;

	if (api == NULL) {
		SYS_LOG_ERR("Could not bind to device API.");
		return NULL;
	}

	/* k_fifo_get returns pointer to a net_pkt object */
	return api->rx(dev, timeout);
}

/** Clear Channel Assesment - Check channel's activity */
__syscall int radio_cca(struct device *dev);

static inline int radio_cca(struct device *dev)
{
	const struct radio_api *api = dev->driver_api;

	if (api == NULL) {
		SYS_LOG_ERR("Could not bind to device API.");
		return -EFAULT;
	}

	return api->cca(dev);
}

/** Set current channel */
__syscall int radio_set_channel(struct device *dev, u16_t channel);

static inline int radio_set_channel(struct device *dev, u16_t channel)
{
	const struct radio_api *api = dev->driver_api;

	if (api == NULL) {
		SYS_LOG_ERR("Could not bind to device API.");
		return -EFAULT;
	}

	return api->set_channel(dev, channel);
}

/** Set address filters (for IEEE802154_HW_FILTER ) */
__syscall int radio_set_filter(struct device *dev,
		  enum ieee802154_filter_type type,
		  const struct ieee802154_filter *filter);

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

/** Set TX power level in dbm */
__syscall int radio_set_txpower(struct device *dev, s16_t dbm);

static inline int radio_set_txpower(struct device *dev, s16_t dbm)
{
	const struct radio_api *api = dev->driver_api;

	if (api == NULL) {
		SYS_LOG_ERR("Could not bind to device API.");
		return -EFAULT;
	}

	return api->set_txpower(dev, dbm);
}

/** Start the device */
__syscall int radio_start(struct device *dev);

static inline int radio_start(struct device *dev)
{
	const struct radio_api *api = dev->driver_api;

	if (api == NULL) {
		SYS_LOG_ERR("Could not bind to device API.");
		return -EFAULT;
	}

	return api->start(dev);
}

/** Stop the device */
__syscall int radio_stop(struct device *dev);

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
