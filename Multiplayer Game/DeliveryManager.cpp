#include "Networks.h"
#include "DeliveryManager.h"

// TODO(you): Reliability on top of UDP lab session

Delivery* DeliveryManager::writeSequenceNumber(OutputMemoryStream& packet)
{
	packet << nextSequenceNumber;

	Delivery* delivery = new Delivery();
	delivery->sequenceNumber = nextSequenceNumber++;
	delivery->dispatchTime = Time.time;

	pendingDeliveries.push_back(delivery);

	return delivery;
}

bool DeliveryManager::processSequenceNumber(const InputMemoryStream& packet)
{
	uint32 sequenceNumber;
	packet >> sequenceNumber;

	// Check seq number in order
	if (sequenceNumber == nextExpectedSequenceNumber)
	{
		pendingAck.push_back(sequenceNumber);
		nextExpectedSequenceNumber++;
		return true;
	}
	return false;
}

void DeliveryManager::writeSequenceNumbersPendingAck(OutputMemoryStream& packet)
{
	for (int i = 0; i < pendingAck.size(); ++i)
	{
		packet << pendingAck[i];
	}

	pendingAck.clear();
}

void DeliveryManager::processAckdSequenceNumbers(const InputMemoryStream& packet)
{
	while ((int)packet.RemainingByteCount() > 0)
	{
		uint32 sequenceNumber = 0;
		packet >> sequenceNumber;

		for (auto it = pendingDeliveries.begin(); it != pendingDeliveries.end();)
		{
			if ((*it)->sequenceNumber == sequenceNumber)
			{
				//(*it)->delegate->OnDeliverySucces(this);
				delete* it;
				it = pendingDeliveries.erase(it);
			}
			else
				++it;
		}
	}
}

void DeliveryManager::processTimedOutPackets()
{
	for (auto it = pendingDeliveries.begin(); it != pendingDeliveries.end();)
	{
		if (Time.time - (*it)->dispatchTime >= PACKET_DELIVERY_TIMEOUT_SECONDS)
		{
			//(*it)->delegate->OnDeliveryFailure(this);
			delete* it;
			it = pendingDeliveries.erase(it);
		}
		else
			++it;
	}
}
