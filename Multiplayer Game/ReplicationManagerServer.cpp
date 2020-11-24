#include "Networks.h"
#include "ReplicationManagerServer.h"
#include "ModuleNetworking.h"

// TODO(you): World state replication lab session

void ReplicationManagerServer::create(uint32 networkId)
{
	ReplicationCommand repCommand;
	repCommand.action = ReplicationAction::Create;
	repCommand.networkId = networkId;
	repCommands.insert(std::pair<uint32, ReplicationCommand>(networkId, repCommand));
}

void ReplicationManagerServer::update(uint32 networkId)
{
	repCommands[networkId].action = ReplicationAction::Update;
}

void ReplicationManagerServer::destroy(uint32 networkId)
{
	repCommands[networkId].action = ReplicationAction::Destroy;
}

void ReplicationManagerServer::write(OutputMemoryStream& packet)
{
	packet << ServerMessage::Replication;

	for(auto it = repCommands.begin(); it != repCommands.end(); ++it)
	{
		packet << (*it).first;
		packet << (*it).second.action;

		switch ((*it).second.action)
		{
		case ReplicationAction::Create:
		{
			GameObject* GO = App->modLinkingContext->getNetworkGameObject((*it).first);
			// Serialize GO
			GO->write(packet);
			GO->sprite->write(packet);

		}
		break;
		case ReplicationAction::Update:
		{
			GameObject* GO = App->modLinkingContext->getNetworkGameObject((*it).first);
			// Serialize position, angle, collider, behaviour,
			GO->write(packet);
		}
		break;
		case ReplicationAction::Destroy:
			break;
		}

		// Clear/Remove the replication command
		
	}
	repCommands.clear();
}
