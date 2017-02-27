/*
 * utilities.cpp
 *
 * Created: 18.02.2013 14:35:48
 *  Author: JochenAlt
 */ 

#include "Arduino.h"
#include "MsgType.h"

Stream* Datagram::errorStream = NULL;
Stream* StreamComm::errorStream = NULL;

/*
void testMessageType() {
	Datagram::setErrorStream(&Serial);
	
	// create message
	Datagram msg1;
	uint8_t idx = 0;
	msg1.setMessageHeader(10,40000);
	msg1.setMessageDataInt16(idx,-20000);
	msg1.setMessageDataInt16(idx,-20000);
	msg1.setMessageDataInt16(idx,+180);
	msg1.setMessageData(idx,180);

	msg1.computeChecksum();
	
	// send
	char buffer[100];
	for (int i = 0;i<msg1.getRawMessageLength();i++) {
		buffer[i] = msg1.getRawMessageData(i);
	}
	
	// parse
	Datagram msg2;
	if (msg2.isMessage(buffer[0])) {
		int i = 1;
		while (msg2.setRawData(buffer[i++]));
		uint8_t idx = 0;
		int16_t x = msg1.getMessageDataInt16(idx);
		x = msg1.getMessageDataInt16(idx);
		x = msg1.getMessageData(idx);

		bool ok = msg2.isChecksumOk();
		bool s = ok;
	}
}
*/

void ControlConfigurationType::initDefaultValues() {
	angleWeight_fp8				= FLOAT2FP16(39.0,8);  
	angularSpeedWeight_fp8		= FLOAT2FP16(21.00,8);
	positionWeight_fp10			= FLOAT2FP16(1.5,10);   
	velocityWeight_fp10			= FLOAT2FP16(0.0,10);  
	accelWeight_fp7				= FLOAT2FP16(1.3,7);   
	bodyPositionWeight_fp10		= FLOAT2FP16(0.0,10);   
	bodyVelocityWeight_fp10		= FLOAT2FP16(9.0,10);  
	bodyAccelWeight_fp7			= FLOAT2FP16(0.0,7); 
	omegaWeight_fp7				= FLOAT2FP16(0.0,7);		
}

void ControlConfigurationType::null() {
	angleWeight_fp8				= FLOAT2FP16(0.0,8);  
	angularSpeedWeight_fp8		= FLOAT2FP16(0.00,8);
	positionWeight_fp10			= FLOAT2FP16(0.0,10);   
	velocityWeight_fp10			= FLOAT2FP16(0.0,10);  
	accelWeight_fp7				= FLOAT2FP16(0.0,7);   
	bodyPositionWeight_fp10		= FLOAT2FP16(0.0,10);   
	bodyVelocityWeight_fp10		= FLOAT2FP16(0.0,10);  
	bodyAccelWeight_fp7			= FLOAT2FP16(0.0,7); 
	omegaWeight_fp7				= FLOAT2FP16(0.0,7);		
}

void ControlConfigurationType::print() {
	Serial.print(F("control weights : angle=("));		
	Serial.print(FP2FLOAT(angleWeight_fp8,8),2,2);Serial.print(",");
	Serial.print(FP2FLOAT(angularSpeedWeight_fp8,8),2,2);Serial.print(F(")"));

	Serial.print(F(" base=("));
	Serial.print(FP2FLOAT(positionWeight_fp10,10),2,1);Serial.print(",");
	Serial.print(FP2FLOAT(velocityWeight_fp10,10),2,1);Serial.print(",");
	Serial.print(FP2FLOAT(accelWeight_fp7,7),2,1);
	Serial.print(F(") body=("));
	Serial.print(FP2FLOAT(bodyPositionWeight_fp10,10),2,1);Serial.print(",");
	Serial.print(FP2FLOAT(bodyVelocityWeight_fp10,10),2,1);Serial.print(",");
	Serial.print(FP2FLOAT(bodyAccelWeight_fp7,7),2,1);Serial.print(")");
	Serial.print(F(" omega= ("));		 
	Serial.print(FP2FLOAT(omegaWeight_fp7,7),2,1);Serial.print(")");
	Serial.println();
}	



Datagram::Datagram () {
	sent_ms = 0;
}

// set message type and sequence number
void Datagram::setMessageHeader(uint8_t pMessageType, uint16_t pSequenceNumber) {
	messageType = pMessageType;
	sequenceNumber = pSequenceNumber;
	checkSum = 0;
	messageLength = 0;
}

// set content
void Datagram::setMessageDataInt8(uint8_t &pIdx, uint8_t pData) {			
	if (pIdx>MESSAGE_MAX_SIZE) {
		setErrorLamp(NULL);
		errorStream->print(F("err:Msg:set idx out of bounds"));	
		errorStream->println(pIdx);
	}
	if (messageLength<=pIdx)
		messageLength = pIdx+1;
	message[pIdx++] = pData;	
}

uint8_t Datagram::getMessageDataInt8(uint8_t &pIdx) {			
	if (pIdx>MESSAGE_MAX_SIZE) {
		errorStream->print(F("err:Msg:get idx out of bounds"))	;
		errorStream->println(pIdx);
	}
	return message[pIdx++];
}

		
uint8_t Datagram::getChecksum() {
	uint8_t localCheckSum = 0;
	localCheckSum = messageType + sequenceNumber + messageLength;
	for (int i = 0;i<messageLength;i++) {
		localCheckSum = localCheckSum + message[i];
	}
	return localCheckSum;
}

// return size of raw message 
uint8_t Datagram::getRawMessageLength() {
	return 1 + sizeof(messageType) + sizeof(sequenceNumber) + sizeof(checkSum) + sizeof(messageLength) + messageLength;
}

		
uint8_t Datagram::getRawMessageData(uint8_t pIdx) {
	switch (pIdx) {
		case 0:	return MESSAGE_MAGIC_NUMBER; // indicate of a message
		case 1: return messageType;
		case 2: return sequenceNumber >> 8;
		case 3: return sequenceNumber & 0xFF;
		case 4: return checkSum;
		case 5: return messageLength;
		default:
			uint8_t idx = pIdx-6;
			return getMessageDataInt8(idx);
	}
}						
				
// returns true, if the passed character indicates a message
bool Datagram::isMessage(char pFirstByte) {
	if (pFirstByte == MESSAGE_MAGIC_NUMBER) {
		messageType = 0;
		sequenceNumber = 0;
		checkSum = 0;
		messageLength = 0;
		rawCounter = 1;		
		return true;
	}
	return false;
}

// returns true, if the passed character indicates a message
bool Datagram::setRawData(char pNextByte) {
	switch (rawCounter) {
		case 1:
			messageType = pNextByte;
			break;
		case 2:
			sequenceNumber = ((uint8_t)pNextByte) << 8;
			break;
		case 3:
			sequenceNumber = sequenceNumber + ((uint8_t)pNextByte);
			break;
		case 4:
			checkSum = (uint8_t)pNextByte;
			break;
		case 5:
			messageLength = ((uint8_t)pNextByte);
			if (messageLength == 0)
				return false;
			break;
		default:
			if (rawCounter-6<=MESSAGE_MAX_SIZE) {
				message[rawCounter-6] = pNextByte;
				rawCounter++;
				return (rawCounter-6 < messageLength);
			}						
			break;
	}
	rawCounter++;
	return true;
}



StreamComm::StreamComm() {
	errorStream = &Serial;	
}
		
// send a message to a stream
void StreamComm::send(Stream* pSerial, Datagram &pMsg) {
	// compute checksum before sending
	pMsg.computeChecksum();
	// Serial.print(F("send:"));
	// pMsg.print();
	
			
	for (int i = 0;i<pMsg.getRawMessageLength();i++)
		pSerial->print(char(pMsg.getRawMessageData(i)));
		
	// set time when this call was done
	pMsg.setSentTime();
}


void Datagram::print() {
	Serial.print(F("datagram=(chk="));
	Serial.print(checkSum);
	Serial.print(F(";txt="));
	for (int i = 0;i<getRawMessageLength();i++) {
		Serial.print('(');
		Serial.print((int)char(getRawMessageData(i)));
		Serial.print(')');
	}		
	Serial.println(F(")"));
}

// send a message to a stream
void StreamComm::send(Stream* pSerial, Datagram &pMsg, uint8_t len, char* pText) {
	// compute checksum before sending
	pMsg.computeChecksum();
	
	// Serial.print(F("sentt:"));
	// pMsg.print();
			
	for (int i = 0;i<pMsg.getRawMessageLength();i++)
		pSerial->print(char(pMsg.getRawMessageData(i)));
	
	// send the string as well
	for (int i = 0;i<len;i++) {
		pSerial->print(pText[i]);	
	}
	// set time when this call was done
	pMsg.setSentTime();
}
				

// if a char has been read, the first character is used to check the 
// magic character whether it is a MessageType or something else
bool StreamComm::isMessage(char pFirstChar, Datagram &pMsg) {
	return pMsg.isMessage(pFirstChar);
}		
		
// read a message from a stream, assuming that the first char has already been read
// returns true if a valid message has been read
bool StreamComm::receive(Stream* pSerial, char pFirstChar, uint32_t pTimeoutUs, Datagram &pMsg) {
	char inputChar;
	bool continueReading;
	uint32_t start = micros(); 
	do {
		continueReading = false; // assume to quit
		
		// wait until sign is available
		if (readFromSerial(pSerial, pTimeoutUs, inputChar)) {
			continueReading = pMsg.setRawData(inputChar);
		}
		else {					
			setErrorLamp(NULL);
			errorStream->print(F("err:R:timeout("));
			errorStream->print(pTimeoutUs);
			errorStream->print(F("us) t="));
			errorStream->print(micros()-start);
			errorStream->print(F("us"));
			// pMsg.print();

			return false; // timeout!
		}
	}				
	while (continueReading);
	if (!pMsg.isChecksumOk()) {
		setErrorLamp(NULL);
		errorStream->print(F("err:checksum"));
		errorStream->print(pTimeoutUs);
		errorStream->print(F("us chk!="));
		errorStream->print(pMsg.getChecksum());
		pMsg.print();
		return false;
	}
	return true;
}

bool StreamComm::receiveVarString(Stream* pSerial, uint32_t pTimeoutUs, uint8_t pLen, char* pText) {
	char inputChar;
	bool continueReading;
	uint8_t idx = 0;
	if (pLen == 0) {
		if (pText != NULL)
			pText[0] = 0;
		return true;
	}
	do {
		continueReading = false; // assume to quit
		
		// wait until sign is available
		if (readFromSerial(pSerial, pTimeoutUs, inputChar)) {
			pText[idx++] = inputChar;
			pText[idx] = 0; // in case the loop stops due to an error, there's always a \0

			continueReading = (idx<pLen);
		}
		else {					
			setErrorLamp(NULL);
			errorStream->print(F("err:RV:timeout len="));
			errorStream->print(pLen);
			errorStream->print(F(" idx="));
			errorStream->print(idx);
			
			return false; // timeout!
		}
	}				
	while (continueReading);
	return true;
}
		
boolean StreamComm::readFromSerial(Stream* pSerial, uint32_t pTimeoutUs, char &pChar) {
	pChar = 0;

	if (pSerial->available()) {
		pChar = pSerial->read();
		return true;
	}
	
	uint32_t start = micros();
	while (!pSerial->available() && ((micros()-start)<pTimeoutUs));

	if (!pSerial->available()) 
		return false;
		
	pChar = pSerial->read();
	return true;
}
