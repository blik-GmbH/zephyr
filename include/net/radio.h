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
	struct radio_api *api = dev->driver_api;

	if (api) {
		return api->get_capabilities(dev);
	}

	SYS_LOG_ERR("Could not bind to device API.");
	return 0;
}

/** Transmit a packet fragment */
__syscall int radio_send(struct device *dev, struct net_buf_simple *frag);

static inline int radio_send(struct device *dev,
		struct net_buf_simple *frag)
{
	struct radio_api *api = dev->driver_api;

	if (api) {
		return api->tx(dev, frag);
	}

	SYS_LOG_ERR("Could not bind to device API.");
	return -EFAULT;
}

/** A fifo for the driver to write RX packages into */
__syscall struct net_buf_simple *radio_recv(struct device *dev,
		u32_t timeout);

static inline struct net_buf_simple *radio_recv(struct device *dev,
		u32_t timeout)
{
	struct radio_api *api = dev->driver_api;

	if (api) {
		/* k_fifo_get returns pointer to a net_pkt object */
		return api->rx(dev, timeout);
	}

	SYS_LOG_ERR("Could not bind to device API.");
	return NULL;
}

/** Clear Channel Assesment - Check channel's activity */
__syscall int radio_cca(struct device *dev);

static inline int radio_cca(struct device *dev)
{
	struct radio_api *api = dev->driver_api;

	if (api) {
		return api->cca(dev);
	}

	SYS_LOG_ERR("Could not bind to device API.");
	return -EFAULT;
}

/** Set current channel */
__syscall int radio_set_channel(struct device *dev, u16_t channel);

static inline int radio_set_channel(struct device *dev, u16_t channel)
{
	struct radio_api *api = dev->driver_api;

	if (api) {
		return api->set_channel(dev, channel);
	}

	SYS_LOG_ERR("Could not bind to device API.");
	return -EFAULT;
}

/** Set address filters (for IEEE802154_HW_FILTER ) */
__syscall int radio_set_filter(struct device *dev,
		  enum ieee802154_filter_type type,
		  const struct ieee802154_filter *filter);

static inline int radio_set_filter(struct device *dev,
		  enum ieee802154_filter_type type,
		  const struct ieee802154_filter *filter)
{
	struct radio_api *api = dev->driver_api;

	if (api) {
		return api->set_filter(dev, type, filter);
	}

	SYS_LOG_ERR("Could not bind to device API.");
	return -EFAULT;
}

/** Set TX power level in dbm */
__syscall int radio_set_txpower(struct device *dev, s16_t dbm);

static inline int radio_set_txpower(struct device *dev, s16_t dbm)
{
	struct radio_api *api = dev->driver_api;

	if (api) {
		return api->set_txpower(dev, dbm);
	}

	SYS_LOG_ERR("Could not bind to device API.");
	return -EFAULT;
}

/** Start the device */
__syscall int radio_start(struct device *dev);

static inline int radio_start(struct device *dev)
{
	struct radio_api *api = dev->driver_api;

	if (api) {
		return api->start(dev);
	}

	SYS_LOG_ERR("Could not bind to device API.");
	return -EFAULT;
}

/** Stop the device */
__syscall int radio_stop(struct device *dev);

static inline int radio_stop(struct device *dev)
{
	struct radio_api *api = dev->driver_api;

	if (api) {
		return api->stop(dev);
	}

	SYS_LOG_ERR("Could not bind to device API.");
	return -EFAULT;
}

#ifdef __cplusplus
}
#endif
