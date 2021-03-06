#include "ModuleNetworkingServer.h"
#include <math.h>


//////////////////////////////////////////////////////////////////////
// ModuleNetworkingServer public methods
//////////////////////////////////////////////////////////////////////

void ModuleNetworkingServer::setListenPort(int port)
{
	listenPort = port;
}



//////////////////////////////////////////////////////////////////////
// ModuleNetworking virtual methods
//////////////////////////////////////////////////////////////////////

void ModuleNetworkingServer::onStart()
{
	if (!createSocket()) return;

	// Reuse address
	int enable = 1;
	int res = setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(int));
	if (res == SOCKET_ERROR) {
		reportError("ModuleNetworkingServer::start() - setsockopt");
		disconnect();
		return;
	}

	// Create and bind to local address
	if (!bindSocketToPort(listenPort)) {
		return;
	}

	state = ServerState::Listening;
}

void ModuleNetworkingServer::onGui()
{
	if (ImGui::CollapsingHeader("ModuleNetworkingServer", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Connection checking info:");
		ImGui::Text(" - Ping interval (s): %f", PING_INTERVAL_SECONDS);
		ImGui::Text(" - Disconnection timeout (s): %f", DISCONNECT_TIMEOUT_SECONDS);

		ImGui::Separator();

		if (state == ServerState::Listening)
		{
			int count = 0;

			for (int i = 0; i < MAX_CLIENTS; ++i)
			{
				if (clientProxies[i].connected)
				{
					ImGui::Text("CLIENT %d", count++);
					ImGui::Text(" - address: %d.%d.%d.%d",
						clientProxies[i].address.sin_addr.S_un.S_un_b.s_b1,
						clientProxies[i].address.sin_addr.S_un.S_un_b.s_b2,
						clientProxies[i].address.sin_addr.S_un.S_un_b.s_b3,
						clientProxies[i].address.sin_addr.S_un.S_un_b.s_b4);
					ImGui::Text(" - port: %d", ntohs(clientProxies[i].address.sin_port));
					ImGui::Text(" - name: %s", clientProxies[i].name.c_str());
					ImGui::Text(" - id: %d", clientProxies[i].clientId);
					if (clientProxies[i].gameObject != nullptr)
					{
						ImGui::Text(" - gameObject net id: %d", clientProxies[i].gameObject->networkId);
					}
					else
					{
						ImGui::Text(" - gameObject net id: (null)");
					}
					
					ImGui::Separator();
				}
			}

			ImGui::Checkbox("Render colliders", &App->modRender->mustRenderColliders);
		}
	}
}

void ModuleNetworkingServer::onPacketReceived(const InputMemoryStream &packet, const sockaddr_in &fromAddress)
{
	if (state == ServerState::Listening)
	{
		uint32 protoId;
		packet >> protoId;
		if (protoId != PROTOCOL_ID) return;

		ClientMessage message;
		packet >> message;

		ClientProxy *proxy = getClientProxy(fromAddress);

		if (message == ClientMessage::Hello)
		{
			if (proxy == nullptr)
			{
				proxy = createClientProxy();

				if (proxy != nullptr)
				{
					std::string playerName;
					uint8 spaceshipType;
					packet >> playerName;
					packet >> spaceshipType;

					proxy->address.sin_family = fromAddress.sin_family;
					proxy->address.sin_addr.S_un.S_addr = fromAddress.sin_addr.S_un.S_addr;
					proxy->address.sin_port = fromAddress.sin_port;
					proxy->connected = true;
					proxy->name = playerName;
					proxy->clientId = nextClientId++;

					// Create new network object
					vec2 initialPosition = 500.0f * vec2{ Random.next() - 0.5f, Random.next() - 0.5f};
					float initialAngle = 360.0f * Random.next();
					proxy->gameObject = spawnPlayer(spaceshipType, initialPosition, initialAngle);
				}
				else
				{
					// NOTE(jesus): Server is full...
				}
			}

			if (proxy != nullptr)
			{
				// Send welcome to the new player
				OutputMemoryStream welcomePacket;
				welcomePacket << PROTOCOL_ID;
				welcomePacket << ServerMessage::Welcome;
				welcomePacket << proxy->clientId;
				welcomePacket << proxy->gameObject->networkId;
				welcomePacket << isGame;
				welcomePacket << SendScore(proxy->name.c_str());
				sendPacket(welcomePacket, fromAddress);

				// Send all network objects to the new player
				uint16 networkGameObjectsCount;
				GameObject *networkGameObjects[MAX_NETWORK_OBJECTS];
				App->modLinkingContext->getNetworkGameObjects(networkGameObjects, &networkGameObjectsCount);
				for (uint16 i = 0; i < networkGameObjectsCount; ++i)
				{
					GameObject *gameObject = networkGameObjects[i];
					
					// TODO(you): World state replication lab session
					proxy->repManagerServer.create(gameObject->networkId);
				}

				LOG("Message received: hello - from player %s", proxy->name.c_str());
			}
			else
			{
				OutputMemoryStream unwelcomePacket;
				unwelcomePacket << PROTOCOL_ID;
				unwelcomePacket << ServerMessage::Unwelcome;
				sendPacket(unwelcomePacket, fromAddress);

				WLOG("Message received: UNWELCOMED hello - server is full");
			}
		}
		else if (message == ClientMessage::Input)
		{
			// Process the input packet and update the corresponding game object
			if (proxy != nullptr && IsValid(proxy->gameObject))
			{
				// TODO(you): Reliability on top of UDP lab session

				// Read input data
				while (packet.RemainingByteCount() > 0)
				{
					InputPacketData inputData;
					packet >> inputData.sequenceNumber;
					packet >> inputData.horizontalAxis;
					packet >> inputData.verticalAxis;
					packet >> inputData.buttonBits;

					if (inputData.sequenceNumber >= proxy->nextExpectedInputSequenceNumber 
						/*&& inputData.sequenceNumber > proxy->lastInputSequenceNumber*/)
					{
						proxy->gamepad.horizontalAxis = inputData.horizontalAxis;
						proxy->gamepad.verticalAxis = inputData.verticalAxis;
						unpackInputControllerButtons(inputData.buttonBits, proxy->gamepad);
						proxy->gameObject->behaviour->onInput(proxy->gamepad);
						proxy->nextExpectedInputSequenceNumber = inputData.sequenceNumber + 1;
					}
				}
			}
		}
		else if (message == ClientMessage::Ping)
		{
			if (proxy != nullptr)
			{
				LOG("Ping Packet Recieved owo");
				proxy->timeSinceLastPacketRecieved = 0.0f;
				proxy->deliveryManager.processAckdSequenceNumbers(packet);
			}
		}
		else if (message == ClientMessage::StartGame)
		{
			OutputMemoryStream packet;
			packet << PROTOCOL_ID;
			packet << ServerMessage::StartGame;
			packet << MAX_HEALTH;

			for (ClientProxy& clientProxy : clientProxies)
			{
				if (clientProxy.connected)
				{
					sendPacket(packet, clientProxy.address);
				}
			}
			StartGame();
		}
		else if (message == ClientMessage::Disconnected)
		{
			int playerScore;
			packet >> playerScore;
			std::string playerName;
			packet >> playerName;

			if (!isScoreRegistered(playerName.c_str(), playerScore))
			{
				score_players.insert(std::pair<std::string, int>(playerName, playerScore));
			}
		}
		else if (message == ClientMessage::Respawn)
		{
			currentHealth--;

			if (currentHealth >= 0)
			{
				OutputMemoryStream packet;
				packet << PROTOCOL_ID;
				packet << ServerMessage::LostHP;
				packet << currentHealth;
				sendPacket(packet, fromAddress);

				for (ClientProxy& clientProxy : clientProxies)
				{
					if (clientProxy.connected)
					{
						sendPacket(packet, clientProxy.address);
					}
				}
			}

			Spaceship* spaceshipBehaviour = (Spaceship*)proxy->gameObject->behaviour;
			spaceshipBehaviour->Respawn();
		}
		else if (message == ClientMessage::LostHP)
		{

		}
		else if (message == ClientMessage::LostGame)
		{
			uint16 netGameObjectsCount;
			GameObject* netGameObjects[MAX_NETWORK_OBJECTS];
			App->modLinkingContext->getNetworkGameObjects(netGameObjects, &netGameObjectsCount);
			for (uint32 i = 0; i < netGameObjectsCount; ++i)
			{
				if (netGameObjects[i]->behaviour && netGameObjects[i]->behaviour->type() != BehaviourType::Spaceship)
					NetworkDestroy(netGameObjects[i]);
			}

			for (ClientProxy& clientProxy : clientProxies)
			{
				if (clientProxy.connected)
				{
					Spaceship* spaceshipBehaviour = (Spaceship*)clientProxy.gameObject->behaviour;
					if (spaceshipBehaviour)
						spaceshipBehaviour->Respawn();
				}
			}

			GameObject* lostMsg = NetworkInstantiate();
			lostMsg->sprite = App->modRender->addSprite(lostMsg);
			lostMsg->sprite->texture = App->modResources->lost;
			lostMsg->sprite->order = 100;

			NetworkDestroy(lostMsg, 1.0f);

			isGame = false;

			OutputMemoryStream packet;
			packet << PROTOCOL_ID;
			packet << ServerMessage::FinishGame;
			sendPacket(packet, fromAddress);
		}

	}
}

void ModuleNetworkingServer::onUpdate()
{
	if (state == ServerState::Listening)
	{
		// Handle networked game object destructions
		for (DelayedDestroyEntry &destroyEntry : netGameObjectsToDestroyWithDelay)
		{
			if (destroyEntry.object != nullptr)
			{
				destroyEntry.delaySeconds -= Time.deltaTime;
				if (destroyEntry.delaySeconds <= 0.0f)
				{
					destroyNetworkObject(destroyEntry.object);
					destroyEntry.object = nullptr;
				}
			}
		}

		// Increase secondsLastPackageTime
		secondsSinceLastPing += Time.deltaTime;
		secondsSinceLastSentPackage += Time.deltaTime;

		for (ClientProxy &clientProxy : clientProxies)
		{
			if (clientProxy.connected)
			{
				// Send Ping Packet
				if (secondsSinceLastPing >= PING_INTERVAL_SECONDS)
				{
					OutputMemoryStream packet;
					packet << PROTOCOL_ID;
					packet << ServerMessage::Ping;
					sendPacket(packet, clientProxy.address);

					if (totalMeteorites <= 0 && isGame)
					{
						OutputMemoryStream packet;
						packet << PROTOCOL_ID;
						packet << ServerMessage::FinishGame;
						WinGame();
						sendPacket(packet, clientProxy.address);
					}
				}

				// Don't let the client proxy point to a destroyed game object
				if (!IsValid(clientProxy.gameObject))
				{
					clientProxy.gameObject = nullptr;
				}

				// TODO(you): World state replication lab session
				if (secondsSinceLastSentPackage >= MAX_NETWORK_DELAY_SECONDS)
				{
					OutputMemoryStream packet;
					packet << PROTOCOL_ID;
					packet << ServerMessage::Replication;
					if(clientProxy.nextExpectedInputSequenceNumber != 0)
						packet << clientProxy.nextExpectedInputSequenceNumber - 1;
					else
						packet << clientProxy.nextExpectedInputSequenceNumber;
					Delivery* delivery = clientProxy.deliveryManager.writeSequenceNumber(packet);

					if (delivery)
					{
						delivery->delegate = new RepDeliveryDelegate(&clientProxy.repManagerServer);
					}
					clientProxy.repManagerServer.write(packet);

					sendPacket(packet, clientProxy.address);
				}

				// Increase timeSinceLastPacketRecieved
				clientProxy.timeSinceLastPacketRecieved += Time.deltaTime;

				if (clientProxy.timeSinceLastPacketRecieved >= DISCONNECT_TIMEOUT_SECONDS)
				{
					destroyClientProxy(&clientProxy);
				}
			}
		}

		// Set Ping time to 0
		if (secondsSinceLastPing >= PING_INTERVAL_SECONDS)
			secondsSinceLastPing = 0.0f;
		if (secondsSinceLastSentPackage >= MAX_NETWORK_DELAY_SECONDS)
			secondsSinceLastSentPackage = 0.0f;
		
	}
}

void ModuleNetworkingServer::onConnectionReset(const sockaddr_in & fromAddress)
{
	// Find the client proxy
	ClientProxy *proxy = getClientProxy(fromAddress);

	if (proxy)
	{
		// Clear the client proxy
		destroyClientProxy(proxy);
	}
}

void ModuleNetworkingServer::onDisconnect()
{
	uint16 netGameObjectsCount;
	GameObject *netGameObjects[MAX_NETWORK_OBJECTS];
	App->modLinkingContext->getNetworkGameObjects(netGameObjects, &netGameObjectsCount);
	for (uint32 i = 0; i < netGameObjectsCount; ++i)
	{
		NetworkDestroy(netGameObjects[i]);
	}

	for (ClientProxy &clientProxy : clientProxies)
	{
		destroyClientProxy(&clientProxy);
	}

	for (DelayedDestroyEntry& destroyEntry : netGameObjectsToDestroyWithDelay)
	{
		destroyEntry.delaySeconds = 0.0f;
		destroyEntry.object = nullptr;
	}
	
	nextClientId = 0;
	secondsSinceLastPing = 0;

	state = ServerState::Stopped;
}



//////////////////////////////////////////////////////////////////////
// Client proxies
//////////////////////////////////////////////////////////////////////

ModuleNetworkingServer::ClientProxy * ModuleNetworkingServer::createClientProxy()
{
	// If it does not exist, pick an empty entry
	for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		if (!clientProxies[i].connected)
		{
			return &clientProxies[i];
		}
	}

	return nullptr;
}

ModuleNetworkingServer::ClientProxy * ModuleNetworkingServer::getClientProxy(const sockaddr_in &clientAddress)
{
	// Try to find the client
	for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		if (clientProxies[i].address.sin_addr.S_un.S_addr == clientAddress.sin_addr.S_un.S_addr &&
			clientProxies[i].address.sin_port == clientAddress.sin_port)
		{
			return &clientProxies[i];
		}
	}

	return nullptr;
}

void ModuleNetworkingServer::destroyClientProxy(ClientProxy *clientProxy)
{
	// Destroy the object from all clients
	if (IsValid(clientProxy->gameObject))
	{
		destroyNetworkObject(clientProxy->gameObject);
	}

	clientProxy->deliveryManager.clear();

    *clientProxy = {};
}

bool ModuleNetworkingServer::isScoreRegistered(const char* name, int score)
{
	for (auto it_map = score_players.begin(); it_map != score_players.end();)
	{
		if (strcmp((*it_map).first.c_str(), name) == 0)
		{
			(*it_map).second = score;
			return true;
		}
	}
	return false;
}

int ModuleNetworkingServer::SendScore(const char* name)
{
	for (auto it_map = score_players.begin(); it_map != score_players.end();)
	{
		if (strcmp((*it_map).first.c_str(), name) == 0)
		{
			return (*it_map).second;
		}
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////
// Spawning
//////////////////////////////////////////////////////////////////////

GameObject * ModuleNetworkingServer::spawnPlayer(uint8 spaceshipType, vec2 initialPosition, float initialAngle)
{
	// Create a new game object with the player properties
	GameObject *gameObject = NetworkInstantiate();
	gameObject->position = initialPosition;
	gameObject->size = { 100, 100 };
	gameObject->angle = initialAngle;

	// Create sprite
	gameObject->sprite = App->modRender->addSprite(gameObject);
	gameObject->sprite->order = 5;

	// Create behaviour
	Spaceship* spaceshipBehaviour = App->modBehaviour->addSpaceship(gameObject);

	if (spaceshipType == 0) {
		gameObject->sprite->texture = App->modResources->spacecraft1;
		gameObject->animation = App->modRender->addAnimation(gameObject);
		gameObject->animation->clip = App->modResources->spaceship01Clip;

		// Set Stats Speed
		spaceshipBehaviour->SetAdvanceSpeed(400.0f);
		spaceshipBehaviour->SetRotateSpeed(500.0f);
		spaceshipBehaviour->SetMaxHealth(4);
	}
	else if (spaceshipType == 1) {
		gameObject->sprite->texture = App->modResources->spacecraft2;
		gameObject->animation = App->modRender->addAnimation(gameObject);
		gameObject->animation->clip = App->modResources->spaceship02Clip;

		// Set Stats Tank
		spaceshipBehaviour->SetAdvanceSpeed(200.0f);
		spaceshipBehaviour->SetRotateSpeed(260.0f);
		spaceshipBehaviour->SetMaxHealth(6);
	}

	gameObject->behaviour = spaceshipBehaviour;
	gameObject->behaviour->isServer = true;

	// Create collider
	gameObject->collider = App->modCollision->addCollider(ColliderType::Player, gameObject);
	gameObject->collider->isTrigger = true; // NOTE(jesus): This object will receive onCollisionTriggered events

	return gameObject;
}

void ModuleNetworkingServer::StartGame()
{
	GameObject* meteor = NetworkInstantiate();

	meteor->position = { 0.0f, 0.0f };
	meteor->angle = 0.0f;
	meteor->size = { 200, 200 };

	meteor->sprite = App->modRender->addSprite(meteor);
	meteor->sprite->order = 5;
	meteor->sprite->texture = App->modResources->asteroid1;

	Meteorite* meteroriteBehaviour = App->modBehaviour->addMeteorite(meteor);
	meteroriteBehaviour->isServer = isServer();

	for (int i = 0; i <= meteroriteBehaviour->GetMaxLevel(); i++)
	{
		totalMeteorites += pow(meteroriteBehaviour->GetDivision(), i);
	}
	
	// Create collider
	meteor->collider = App->modCollision->addCollider(ColliderType::Meteorite, meteor);
	meteor->collider->isTrigger = true;

	GameObject* startMsg = NetworkInstantiate();
	startMsg->sprite = App->modRender->addSprite(startMsg);
	startMsg->sprite->texture = App->modResources->start;
	startMsg->sprite->order = 100;

	NetworkDestroy(startMsg, 1.0f);

	currentHealth = MAX_HEALTH;
	isGame = true;
}

void ModuleNetworkingServer::WinGame()
{
	isGame = false;

	GameObject* winMsg = NetworkInstantiate();
	winMsg->sprite = App->modRender->addSprite(winMsg);
	winMsg->sprite->texture = App->modResources->win;
	winMsg->sprite->order = 100;

	NetworkDestroy(winMsg, 1.0f);

}


//////////////////////////////////////////////////////////////////////
// Update / destruction
//////////////////////////////////////////////////////////////////////

GameObject * ModuleNetworkingServer::instantiateNetworkObject()
{
	// Create an object into the server
	GameObject * gameObject = Instantiate();

	// Register the object into the linking context
	App->modLinkingContext->registerNetworkGameObject(gameObject);

	// Notify all client proxies' replication manager to create the object remotely
	for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		if (clientProxies[i].connected)
		{
			// TODO(you): World state replication lab session
			clientProxies[i].repManagerServer.create(gameObject->networkId);
		}
	}

	return gameObject;
}

void ModuleNetworkingServer::updateNetworkObject(GameObject * gameObject)
{
	// Notify all client proxies' replication manager to destroy the object remotely
	for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		if (clientProxies[i].connected)
		{
			// TODO(you): World state replication lab session
			clientProxies[i].repManagerServer.update(gameObject->networkId);
		}
	}
}

void ModuleNetworkingServer::destroyNetworkObject(GameObject * gameObject)
{
	// Notify all client proxies' replication manager to destroy the object remotely
	for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		if (clientProxies[i].connected)
		{
			// TODO(you): World state replication lab session
			clientProxies[i].repManagerServer.destroy(gameObject->networkId);
		}
	}

	// Assuming the message was received, unregister the network identity
	App->modLinkingContext->unregisterNetworkGameObject(gameObject);

	// Finally, destroy the object from the server
	Destroy(gameObject);
}

void ModuleNetworkingServer::destroyNetworkObject(GameObject * gameObject, float delaySeconds)
{
	uint32 emptyIndex = MAX_GAME_OBJECTS;
	for (uint32 i = 0; i < MAX_GAME_OBJECTS; ++i)
	{
		if (netGameObjectsToDestroyWithDelay[i].object == gameObject)
		{
			float currentDelaySeconds = netGameObjectsToDestroyWithDelay[i].delaySeconds;
			netGameObjectsToDestroyWithDelay[i].delaySeconds = min(currentDelaySeconds, delaySeconds);
			return;
		}
		else if (netGameObjectsToDestroyWithDelay[i].object == nullptr)
		{
			if (emptyIndex == MAX_GAME_OBJECTS)
			{
				emptyIndex = i;
			}
		}
	}

	ASSERT(emptyIndex < MAX_GAME_OBJECTS);

	netGameObjectsToDestroyWithDelay[emptyIndex].object = gameObject;
	netGameObjectsToDestroyWithDelay[emptyIndex].delaySeconds = delaySeconds;
}


//////////////////////////////////////////////////////////////////////
// Global create / update / destruction of network game objects
//////////////////////////////////////////////////////////////////////

GameObject * NetworkInstantiate()
{
	ASSERT(App->modNetServer->isConnected());

	return App->modNetServer->instantiateNetworkObject();
}

void NetworkUpdate(GameObject * gameObject)
{
	ASSERT(App->modNetServer->isConnected());
	ASSERT(gameObject->networkId != 0);

	App->modNetServer->updateNetworkObject(gameObject);
}

void NetworkDestroy(GameObject * gameObject)
{
	NetworkDestroy(gameObject, 0.0f);
}

void NetworkDestroy(GameObject * gameObject, float delaySeconds)
{
	ASSERT(App->modNetServer->isConnected());
	ASSERT(gameObject->networkId != 0);

	App->modNetServer->destroyNetworkObject(gameObject, delaySeconds);
}
