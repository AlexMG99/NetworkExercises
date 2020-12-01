#pragma once

// TODO(you): Reliability on top of UDP lab session
class DeliveryManager;

class DeliveryDelegate
{
public:
	virtual void OnDeliverySucces(DeliveryManager* deliveryManager) = 0;
	virtual void OnDeliveryFailure(DeliveryManager* deliveryManager) = 0;
};

struct Delivery
{
	uint32 sequenceNumber = 0;
	double dispatchTime = 0;
	DeliveryDelegate* delegate = nullptr;
};

class DeliveryManager
{
public:
	// For senders to write a new seq numbers into a packet
	Delivery* writeSequenceNumber(OutputMemoryStream &packet);

	// For recievers to process the seq number from an incoming packet
	bool processSequenceNumber(const InputMemoryStream& packet);

	// For receivers to write ack'ed seq numbers into a packet
	bool hasSequenceNumbersPendingAck() const;
	void writeSequenceNumbersPendingAck(OutputMemoryStream& packet);

	// For senders to process ack'ed seq numbers from a packet
	void processAckdSequenceNumbers(const InputMemoryStream& packet);
	void processTimedOutPackets();

	void clear();

private:
	// Private members (sender side)
	// - The next outgoing sequence number
	uint32 nextSequenceNumber = 0;
	// - A list of pending deliveries
	std::vector<Delivery*> pendingDeliveries;

	// Private members (reciever side)
	// - The next expected sequence number
	uint32 nextExpectedSequenceNumber = 0;
	// - A list of sequence numbers pending ack
	std::vector<uint32> pendingAck;
};