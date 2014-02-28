/*
 ===============================================================================
 Name        : ble_serial.h
 Author      : Jason
 Version     : 1.0.10
 Date		 : 2014/2/24
 Copyright   : Copyright (C) www.embeda.com.tw
 Description : UART (Serial Stream) service for BLE
 ===============================================================================
 History
 ---------+---------+--------------------------------------------+-------------
 DATE     |	VERSION |	DESCRIPTIONS							 |	By
 ---------+---------+--------------------------------------------+-------------
 2014/1/1	v1.0.0	Initialize										Jason
 2014/1/4	v1.0.1	Add onAckTimeout() event for ACK timeout.		Jason
 2014/1/18	v1.0.2	Rename to bleSerial Class						Jason
 2014/1/19	v1.0.3	Add onAlert / onLinkLose / setTxPowerLevel		Jason
 2014/1/20	v1.0.4	Modify ble engine process for RTOS				Jason
 2014/1/22	v1.0.5	Add parameter 'stack' for enable() member		Jason
 	 	 	 	 	Add available() member to UART service check.
 2014/2/05	v1.0.6	Add isReadyForNotify() member for notify service.
 2014/2/11	v1.0.7	Add setup() member function.					Jason
 2014/2/18	v1.0.8	Add m_semDataCredit semaphore.					Jason
 2014/2/21	v1.0.9	Add connInterval & connTimeout parameters in	Jason
 	 	 	 	 	setup() member function.
 2014/2/24	v1.0.10 Rename setup() to advertising() 				Jason
 ===============================================================================
 */

#ifndef BLE_SERIAL_H
#define BLE_SERIAL_H

#include "class/stream.h"
#include "class/thread.h"
#include "class/spi.h"
#include "class/pin.h"
#include "class/semaphore.h"
#include "class/mutex.h"
#include "class/timeout.h"

typedef enum {
	BLE_TERMINATED =1,
	BLE_UNACCEPTABLE = 2
} BLE_DISCONNECT_REASON_T;

typedef enum {
	BLE_TX_m18dBm 	= 0,	// -18dBm  	(Low)
	BLE_TX_m12dBm 	= 1,	// -12dBm
	BLE_TX_m6dBm	= 2,	// -6dBm
	BLE_TX_0dBm 	= 3		//  0dBm	(High)
}BLE_TX_POWER_T;

#define BLE_RESPONSE_TIMEOUT	3000
#define DEF_BLE_DEVICENAME		"uCXpresso.BLE"
#define DEF_BLE_ADV_INTERVAL	500		// default advertising intervel 500ms
#define DEF_BLE_TXPOWER			-65		// default tx power level -65dbm @ 1M
#define DEF_BLE_NOTIFY_TIMEOUT	3000
#define DEF_BLE_DATA_TIMEOUT	1500
#define DEF_BLE_CONN_INTERVAL	10		// default connection interval 10ms
#define DEF_BLE_CONN_TIMEOUT	1000	// default connection timeout 3sec
#define DEF_BLE_MFG_DATA		0x1234

//
// bleSerial stream class
//
class bleSerial: public CStream, public CThread {
public:
	//
	// construct
	//
	bleSerial(LPCTSTR deviceName=DEF_BLE_DEVICENAME);

	//
	// Advertising setup (optional)
	// Remark:
	// A valid Adv. have to call before the enable() member!!
	//
	void advertising(uint16_t advInterval,					// advInterval : Advertising interval (default 500ms, must >=100ms)
			   int8_t txPowerLevel=DEF_BLE_TXPOWER,			// txPowerLevel: Advertising txPowerLevel (dBm)
			   uint16_t connInterval=DEF_BLE_CONN_INTERVAL,	// connInterval: Connection interval (default 10ms)
			   uint16_t connTimeout=DEF_BLE_CONN_TIMEOUT,	// connTimeout : Connection timeout (default 1000ms)
			   uint16_t manufactureData=DEF_BLE_MFG_DATA);	// manufacture Data

	// redirect to advertising(...)
	inline void setup(uint16_t advInterval,
			   int8_t txPowerLevel=DEF_BLE_TXPOWER,
			   uint16_t connInterval=DEF_BLE_CONN_INTERVAL,
			   uint16_t connTimeout=DEF_BLE_CONN_TIMEOUT,
			   uint16_t manufactureData=DEF_BLE_MFG_DATA) {
		advertising(advInterval, txPowerLevel, connInterval, connTimeout, manufactureData);
	}

	//
	// Controls
	//
#ifdef DEBUG
	bool enable(uint32_t stack=190);
#else
	bool enable(uint32_t stack=128);
#endif
	void disable();

	//
	// Polling Interval (for Power Save)
	//
	void pollInterval(uint32_t ms);	// default 50ms

	//
	// PHY functions
	//
	bool	isActived();	// RF front end activity indicator

	bool	disconnect(BLE_DISCONNECT_REASON_T reason=BLE_TERMINATED);
	bool	setRadioTxPower(BLE_TX_POWER_T power);

	uint8_t getPhyVersion();

	//
	// PHY Events
	//
	virtual void onConnected();
	virtual void onDisconnected();

	//
	// for BLE UART Service
	// Implement the virtual functions of CStream class
	//
	inline bool isAvailable() { return writeable(); }		// check service is available or not
	virtual int	 readable();
	virtual int	 writeable();
	virtual int  read(void *buf, int len, bool block=true);
	virtual int  write(const void *buf, int len, bool block=true);
	virtual bool isConnected();
	virtual void flush();

	//
	// for Immediate Alert Service (for Proximity and internal use)
	//
	virtual void onAlert(uint8_t level);
	virtual void onLoseLink(uint8_t level);
	virtual void setTxPowerLevel(int8_t dBm);	// range: -120 ~ +20 dBm

	//
	// for Heart Rate service (internal use)
	//
	virtual void onHrControl(uint8_t ctrl);

	//
	// PRIVATE, internal used
	//
	virtual ~bleSerial();
			void reset();
	virtual void onResponseCommandHook(void *data);
	virtual void onTransactionFinishedHook();
	virtual void onDataCreditHook();
	virtual void onBleRecv(uint8_t *buf, int len);
	virtual void onBleSend(uint8_t ack);
	virtual void onAckTimeout();
	virtual bool isReadyForNotify();
	LPCTSTR	 m_deviceName;
	uint16_t m_advInterval;
	uint16_t m_conInterval;
	uint16_t m_conTimeout;
	uint16_t m_mfgData;
	int8_t	 m_txPowerLevel;

protected:
	//
	// Implement the virtual function of CThread class
	//
	virtual void run();

protected:
	CMutex			m_mxSender;
	CSemaphore		m_semResponse;
	CSemaphore		m_semTransaction;
	CSemaphore		m_semDataCredit;
	xHandle			m_event;

private:
	CTimeout	m_tmACK;
	CPin		m_active;
	uint32_t	m_pollInterval;

	//
	// Friend Classes
	//
#include "class/ble_battery.h"
#include "class/ble_ht.h"
#include "class/ble_proximity.h"
#include "class/ble_heartrate.h"
#include "class/ble_devinfo.h"
friend class bleBatteryLevel;
friend class bleHealthThermometer;
friend class bleProximity;
friend class bleHeartRate;
friend class bleDeviceInfo;
	class bleProximity	*m_proximity;
	class bleHeartRate	*m_heartrate;

#undef BLE_BATTERY_H_
#undef BLE_HT_H_
#undef BLE_PROXIMITY_H_
#undef BLE_HEARTRATE_H_
#undef BLE_DEVINFO_H_
};

extern bleSerial *objBLE;

#endif /* BLE_SERIAL_H */
