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
			createObject(packet, GO);
		}
		break;
		case ReplicationAction::Update:
		{
			GameObject* GO = App->modLinkingContext->getNetworkGameObject(networkId);
			if(GO)
				GO->read(packet);
		}
		break;
		case ReplicationAction::Destroy:
		{
			GameObject* GO = App->modLinkingContext->getNetworkGameObject(networkId);
			if (GO)
			{
				App->modLinkingContext->unregisterNetworkGameObject(GO);
				App->modGameObject->Destroy(GO);
			}
		}
		break;
		}
	}
}

void ReplicationManagerClient::createObject(const InputMemoryStream& packet, GameObject* go)
{
	// Transform
	go->read(packet);

	// Behaviour
	BehaviourType bType;
	packet >> bType;

	// Texture name
	std::string fName;
	packet >> fName;

	go->sprite = App->modRender->addSprite(go);

	switch (bType)
	{
	case BehaviourType::Spaceship:
	{
		// Create behaviour
		Spaceship* spaceshipBehaviour = App->modBehaviour->addSpaceship(go);
		go->behaviour = spaceshipBehaviour;
		go->behaviour->isServer = false;

		go->sprite->order = 5;

		if (strcmp(fName.c_str(), "spacecraft1.png") == 0)
		{
			go->sprite->texture = App->modResources->spacecraft1;
		}
		else if (strcmp(fName.c_str(), "spacecraft2.png") == 0)
		{
			go->sprite->texture = App->modResources->spacecraft2;
		}
		else if (strcmp(fName.c_str(), "spacecraft3.png") == 0)
		{
			go->sprite->texture = App->modResources->spacecraft3;
		}
	}
	break;
	case BehaviourType::Laser:
	{
		Laser* laserBehaviour = App->modBehaviour->addLaser(go);
		laserBehaviour->isServer = false;

		go->sprite->texture = App->modResources->laser;
		go->sprite->order = 3;
	}
	break;
	case BehaviourType::None:
	{
		LOG("No Type");
	}
	break;
	}
	
	// Collider
	bool hasCollider;
	packet >> hasCollider;

	if (hasCollider)
	{
		ColliderType type;
		packet >> type;

		go->collider = App->modCollision->addCollider(type, go);

		bool isTrigger;
		packet >> isTrigger;

		go->collider->isTrigger = isTrigger;
	}
	

	//// Create sprite
	//gameObject->sprite = App->modRender->addSprite(gameObject);
	//go->sprite->order = 5;
	//if (spaceshipType == 0) {
	//	gameObject->sprite->texture = App->modResources->spacecraft1;
	//}
	//else if (spaceshipType == 1) {
	//	gameObject->sprite->texture = App->modResources->spacecraft2;
	//}
	//else {
	//	gameObject->sprite->texture = App->modResources->spacecraft3;
	//}

	//// Create collider
	//gameObject->collider = App->modCollision->addCollider(ColliderType::Player, gameObject);
	//gameObject->collider->isTrigger = true; // NOTE(jesus): This object will receive onCollisionTriggered events

	//// Create behaviour
	//Spaceship* spaceshipBehaviour = App->modBehaviour->addSpaceship(gameObject);
	//gameObject->behaviour = spaceshipBehaviour;
	//gameObject->behaviour->isServer = true;
}
