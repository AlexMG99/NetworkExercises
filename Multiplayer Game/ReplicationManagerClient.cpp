#include "Networks.h"
#include "ReplicationManagerClient.h"

// TODO(you): World state replication lab session :_)

// Dudas:
//	1. Cuando se dispara el laser, la nave muere. Se borra del cliente, no se deconecta del cliente y en el server no pasa nada
//		1.1 El collider se actualiza solo? Se tiene que mover?
//		1.2 Por que cuando muere no se desconecta? Por que el servidor sigue teniendo nave?
//	2. Cuando desaparece la nave, y se clica un input, el codigo peta, pq la nave esta muerta.
//		2.1 Debería desconectar al cliente del server pq ha muerto.

//	3. No sabemos como debugar si lo del input esta bien

void ReplicationManagerClient::read(const InputMemoryStream& packet)
{
	while ((int)packet.RemainingByteCount() > 0)
	{
		uint32 networkId = 0;
		packet >> networkId;

		ReplicationAction action = ReplicationAction::None;
		packet >> action;

		switch (action)
		{
		case ReplicationAction::None:
			break;
		case ReplicationAction::Create:
		{
			GameObject* GO = App->modGameObject->Instantiate();

			if (App->modLinkingContext->getNetworkGameObject(networkId) != nullptr)
			{
				createObject(packet, GO);
				App->modGameObject->Destroy(GO);
			}
			else
			{
				App->modLinkingContext->registerNetworkGameObjectWithNetworkId(GO, networkId);
				createObject(packet, GO);

				if (networkId == App->modNetClient->GetNetworkId())
					GO->networkInterpolationEnabled = false;
			}
			
		}
		break;
		case ReplicationAction::Update:
		{
			GameObject* GO = App->modLinkingContext->getNetworkGameObject(networkId);
			if (GO)
			{
				GO->read(packet);

				bool hasBehaviour;
				packet >> hasBehaviour;

				if (hasBehaviour)
					GO->behaviour->read(packet);
			}
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
	go->createRead(packet);

	// Texture name
	std::string fName;
	packet >> fName;

	go->sprite = App->modRender->addSprite(go);

	// Behaviour
	bool hasBehaviour;
	packet >> hasBehaviour;
	
	if (hasBehaviour)
	{
		BehaviourType bType;
		packet >> bType;

		// Create behaviour
		Behaviour* behaviour = App->modBehaviour->addBehaviour(bType, go);

		switch (bType)
		{
		case BehaviourType::Spaceship:
		{
			//go->behaviour->isServer = false;

			go->behaviour->read(packet);

			go->sprite->order = 5;

			if (strcmp(fName.c_str(), "spaceship_01.png") == 0)
			{
				go->sprite->texture = App->modResources->spacecraft1;
				go->animation = App->modRender->addAnimation(go);
				go->animation->clip = App->modResources->spaceship01Clip;
			}
			else if (strcmp(fName.c_str(), "spaceship_02.png") == 0)
			{
				go->sprite->texture = App->modResources->spacecraft2;
				go->animation = App->modRender->addAnimation(go);
				go->animation->clip = App->modResources->spaceship02Clip;
			}
			else if (strcmp(fName.c_str(), "spacecraft3.png") == 0)
			{
				go->sprite->texture = App->modResources->spacecraft3;
			}
		}
		break;
		case BehaviourType::Laser:
		{
			//laserBehaviour->isServer = false;

			go->sprite->texture = App->modResources->laser;
			go->sprite->order = 3;
		}
		break;
		case BehaviourType::Meteorite:
		{
			//laserBehaviour->isServer = false;

			go->behaviour->read(packet);

			go->sprite->texture = App->modResources->asteroid1;
			go->sprite->order = 5;
		}
		break;
		case BehaviourType::None:
		{
			LOG("No Type");
		}
		break;
		}
	}

	// Texture
	if (strcmp(fName.c_str(), "explosion1.png") == 0)
	{
		go->sprite->texture = App->modResources->explosion1;
		go->sprite->order = 100;

		go->animation = App->modRender->addAnimation(go);
		go->animation->clip = App->modResources->explosionClip;

		App->modSound->playAudioClip(App->modResources->audioClipExplosion);
	}
	else if (strcmp(fName.c_str(), "start.png") == 0)
	{
		go->sprite->texture = App->modResources->start;
		go->sprite->order = 100;
	}
	else if (strcmp(fName.c_str(), "win.png") == 0)
	{
		//go->sprite->texture = App->modResources->win;
		go->sprite->order = 100;
	}
	else if (strcmp(fName.c_str(), "lost.png") == 0)
	{
		go->sprite->texture = App->modResources->lost;
		go->sprite->order = 100;
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

}
