#include "Networks.h"
#include "ReplicationManagerClient.h"

// TODO(you): World state replication lab session :_)

void ReplicationManagerClient::read(const InputMemoryStream& packet)
{
	while ((int)packet.RemainingByteCount() > 0)
	{
		uint32 networkId;
		packet >> networkId;
		ReplicationAction action;
		packet >> action;

		switch (action)
		{
		case ReplicationAction::Create:
		{
			GameObject* GO = App->modGameObject->Instantiate();
			App->modLinkingContext->registerNetworkGameObjectWithNetworkId(GO, networkId);
			// Deserialize fields
			packet >> GO->position.x;
			packet >> GO->position.y;
			packet >> GO->angle;
		}
		break;
		case ReplicationAction::Update:
		{
			GameObject* GO = App->modLinkingContext->getNetworkGameObject(networkId);
			// Deserialize fields
		}
		break;
		case ReplicationAction::Destroy:
		{
			GameObject* GO = App->modLinkingContext->getNetworkGameObject(networkId);
			App->modLinkingContext->unregisterNetworkGameObject(GO);
			App->modGameObject->Destroy(GO);
		}
		break;
		}
	}
}
