/*
 * MsgType.h
 *
 * Created: 18.02.2013 14:36:02
 * Author: JochenAlt
 * Datagram represent the message content,  in addition it
 *  o computes and checks a checksum
 */ 


#ifndef MSGTYPE_H_
#define MSGTYPE_H_

#include "Arduino.h"
#include "setup.h" // this include gives different definitions in bot board and main board
#include "FixedPoint.h"
#include "TimePassedBy.h"
#include "Paul.h"

// property of communication via these classes
#define MESSAGE_MAX_SIZE 32								// maximum size of user part of a message
#define MESSAGE_MAX_SIZE_RAW (6 + MESSAGE_MAX_SIZE)		// header of each message is 6 chars(incl. magic number)
#define MESSAGE_MAGIC_NUMBER 166						// first character of each message
#define	SET_SPEED_TEXT_SIZE (MESSAGE_MAX_SIZE -12 -1)	// size of communicated spoken text  in buffer


class ControlConfigurationType {
public:
	void null();
	void initDefaultValues();
	void print();
	
	int16_t angleWeight_fp8;
	int16_t angularSpeedWeight_fp8;
	int16_t velocityWeight_fp10;
	int16_t positionWeight_fp10;
	int16_t accelWeight_fp7;
	int16_t bodyVelocityWeight_fp10;
	int16_t bodyPositionWeight_fp10;
	int16_t bodyAccelWeight_fp7;
	int16_t omegaWeight_fp7;
};

// all message types in requests and responses
enum MessageType {	NOMSG,
					// Messages between main board and bot board 
					CALIBRATE_IMU_REQ, CALIBRATE_IMU_RES,	// set offset of IMU
					SET_SPEED_REQ, SET_SPEED_RES,			// set new speed and omega, return current angles and position
					CONFIGURE_REQ, CONFIGURE_RES,			// configure kinematics and controller					

					// messages between remote and main board
					RC_SET_SPEED_REQ, RC_SET_SPEED_RES,			// send balancing on/off, to-be speed, receive angle, position and speed
					RC_CALIBRATE_IMU_REQ, RC_CALIBRATE_IMU_RES,	// calibrate IMU
					RC_CONFIGURE_REQ,RC_CONFIGURE_RES,			// configure control paramter
					RC_SET_OPTIONS_REQ,RC_SET_OPTIONS_RES,		// define volume, voice, etc.
					RC_SPEECH_REQ,RC_SPEECH_RES					// send something to be spoken

};					


class Datagram {
	friend class StreamComm;
	public:
		Datagram ();
		void print();
		// set message type and sequence number
		void setMessageHeader(uint8_t pMessageType, uint16_t pSequenceNumber);
		// set content
		void setMessageDataInt8(uint8_t &pIdx, uint8_t pData);
		uint8_t getMessageDataInt8(uint8_t &pIdx);

		void setMessageDataInt16(uint8_t &pIdx, int16_t pData) {
			setMessageDataInt8(pIdx, pData>>8);
			setMessageDataInt8(pIdx, pData & 0xFF);
		}
		int16_t getMessageDataInt16(uint8_t &pIdx) {
			int16_t result = getMessageDataInt8(pIdx)<< 8;
			result = result + getMessageDataInt8(pIdx);	
			return result ;
		}

		uint8_t getMessageType()	{	return messageType;	}
		uint8_t getSequenceNumber() {	return sequenceNumber;	}
		uint8_t getChecksum();
		void computeChecksum()		{	checkSum = getChecksum(); }
		bool isChecksumOk()			{	return checkSum == getChecksum();	}

		// return size of raw message 
		uint8_t getRawMessageLength();
		// return character of message data at given position, starting at 0	
		uint8_t getRawMessageData(uint8_t pIdx);

		// define where error messages are printed
		static void setErrorStream(Stream* pErrorStream) {
			errorStream = pErrorStream;
		}
		
		// returns true, if the passed character indicates a message
		bool isMessage(char pFirstByte);

		// returns true, if the passed character indicates a message
		bool setRawData(char pNextByte);
		void setSentTime() { sent_ms = millis(); };
		uint16_t getSentTime() { return sent_ms;}; 
		// returns true, if that diagram has been sent too long ago without an reply
		// it is not check wether a reply has been received already
		bool isOverDue(uint16_t pTimeout_ms) {
			return (sent_ms > 0) && ((millis()-sent_ms) >=pTimeout_ms);
		}
	private:
		uint8_t messageType;				// number indicating the method name
		uint16_t sequenceNumber;			// sequence number, optional to be used
		uint8_t checkSum;					// small checksum
		uint8_t messageLength;				// length of user data within message array
	public:
		uint8_t message[MESSAGE_MAX_SIZE];	// the user part of a message
		uint8_t rawCounter;					// counter used during reading, position within the message array
		uint16_t sent_ms;					// time when this datagram has been sent
		static Stream* errorStream;			// static stream where errors are sent to
		
};

// class to send and receive Datagrams across a Stream (Soft- or HardSerial)
class StreamComm {
	friend class Datagram;
	public:
		StreamComm();
		
		// send a message to a stream
		void send(Stream* pSerial, Datagram &pMsg);
		// send a message to a stream with a string of variable length
		void send(Stream* pSerial, Datagram &pMsg, uint8_t len, char* pText);

		// if a char has been read, the first character is used to check the 
		// magic character whether it is a MessageType or something else
		bool isMessage(char pFirstChar, Datagram &pMsg);
		
		// read a message from a stream, assuming that the first char has already been read
		// returns true if a valid message has been read
		bool receive(Stream* pSerial, char pFirstChar, uint32_t pTimeoutUs, Datagram &pMsg);
		bool receiveVarString(Stream* pSerial, uint32_t pTimeout, uint8_t pLen, char* pText);

		// define where error messages are printed
		static void setErrorStream(Stream* pErrorStream) {
			errorStream = pErrorStream;
		};
	private:
		boolean readFromSerial(Stream* pSerial, uint32_t pTimeoutUs, char &pChar);
		static Stream* errorStream;
};


#endif /* MSGTYPE_H_ */