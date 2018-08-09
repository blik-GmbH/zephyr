#pragma once
enum ieee802154_hw_caps {
	IEEE802154_HW_FCS	= BIT(0), /* Frame Check-Sum supported */
	IEEE802154_HW_PROMISC	= BIT(1), /* Promiscuous mode supported */
	IEEE802154_HW_FILTER	= BIT(2), /* Filters PAN ID, long/short addr */
	IEEE802154_HW_CSMA	= BIT(3), /* CSMA-CA supported */
	IEEE802154_HW_2_4_GHZ	= BIT(4), /* 2.4Ghz radio supported */
	IEEE802154_HW_TX_RX_ACK = BIT(5), /* Handles ACK request on TX */
	IEEE802154_HW_SUB_GHZ	= BIT(6), /* Sub-GHz radio supported */
};

enum ieee802154_filter_type {
	IEEE802154_FILTER_TYPE_IEEE_ADDR,
	IEEE802154_FILTER_TYPE_SHORT_ADDR,
	IEEE802154_FILTER_TYPE_PAN_ID,
};

struct ieee802154_filter {
/** @cond ignore */
	union {
		u8_t *ieee_addr;
		u16_t short_addr;
		u16_t pan_id;
	};
/* @endcond */
};

struct radio_api {
	/** A fifo for the driver to write RX packages into */
	//struct k_fifo *rx_queue;
	struct net_buf_simple* (*rx)(struct device *dev, u32_t timeout);

	/** Get the device capabilities */
	enum ieee802154_hw_caps (*get_capabilities)(struct device *dev);

	/** Clear Channel Assesment - Check channel's activity */
	int (*cca)(struct device *dev);

	/** Set current channel */
	int (*set_channel)(struct device *dev, u16_t channel);

	/** Set address filters (for IEEE802154_HW_FILTER ) */
	int (*set_filter)(struct device *dev,
			  enum ieee802154_filter_type type,
			  const struct ieee802154_filter *filter);

	/** Set TX power level in dbm */
	int (*set_txpower)(struct device *dev, s16_t dbm);

	/** Transmit a packet fragment */
	int (*tx)(struct device *dev, struct net_buf_simple *frag);

	/** Start the device */
	int (*start)(struct device *dev);

	/** Stop the device */
	int (*stop)(struct device *dev);
} __packed;

