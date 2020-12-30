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
	if (sequenceNumber >= nextExpectedSequenceNumber)
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
				(*it)->delegate->OnDeliverySuccess(this);
				delete (*it)->delegate;
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
			(*it)->delegate->OnDeliveryFailure(this);
			delete (*it)->delegate;
			delete* it;
			it = pendingDeliveries.erase(it);
		}
		else
			++it;
	}
}

void DeliveryManager::clear()
{
	nextSequenceNumber = 0;

	for (std::vector<Delivery*>::iterator it = pendingDeliveries.begin(); it != pendingDeliveries.end(); ++it)
	{
		(*it)->clear();
		delete (*it);
	}
	pendingDeliveries.clear();

	nextExpectedSequenceNumber = 0;
	pendingAck.clear();
}

RepDeliveryDelegate::RepDeliveryDelegate(ReplicationManagerServer* repManager)
{
	repManagerServer = repManager;
	for (std::unordered_map<uint32, ReplicationCommand>::iterator it_com = repManagerServer->repCommands.begin(); it_com != repManagerServer->repCommands.end(); ++it_com)
	{
		commands.push_back((*it_com).second);
	}
}

void RepDeliveryDelegate::OnDeliveryFailure(DeliveryManager* deliveryManager)
{
	for (std::vector<ReplicationCommand>::iterator it_com = commands.begin(); it_com != commands.end(); ++it_com)
	{
		switch ((*it_com).action)
		{
		case ReplicationAction::Create:
			repManagerServer->create((*it_com).networkId);
			break;
		case ReplicationAction::Update:
			repManagerServer->update((*it_com).networkId);
			break;
		case ReplicationAction::Destroy:
			repManagerServer->destroy((*it_com).networkId);
			break;
		default:
			break;
		}
	}
}
