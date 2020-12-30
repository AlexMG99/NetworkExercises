#include "ModuleNetworkingClient.h"


//////////////////////////////////////////////////////////////////////
// ModuleNetworkingClient public methods
//////////////////////////////////////////////////////////////////////


void ModuleNetworkingClient::setServerAddress(const char * pServerAddress, uint16 pServerPort)
{
	serverAddressStr = pServerAddress;
	serverPort = pServerPort;
}

void ModuleNetworkingClient::setPlayerInfo(const char * pPlayerName, uint8 pSpaceshipType)
{
	playerName = pPlayerName;
	spaceshipType = pSpaceshipType;
}



//////////////////////////////////////////////////////////////////////
// ModuleNetworking virtual methods
//////////////////////////////////////////////////////////////////////

void ModuleNetworkingClient::onStart()
{
	if (!createSocket()) return;

	if (!bindSocketToPort(0)) {
		disconnect();
		return;
	}

	// Create remote address
	serverAddress = {};
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(serverPort);
	int res = inet_pton(AF_INET, serverAddressStr.c_str(), &serverAddress.sin_addr);
	if (res == SOCKET_ERROR) {
		reportError("ModuleNetworkingClient::startClient() - inet_pton");
		disconnect();
		return;
	}

	state = ClientState::Connecting;

	inputDataFront = 0;
	inputDataBack = 0;

	secondsSinceLastPackage = 9999.0f;
	secondsSinceLastInputDelivery = 0.0f;
}

void ModuleNetworkingClient::onGui()
{
	if (state == ClientState::Stopped) return;

	if (ImGui::CollapsingHeader("ModuleNetworkingClient", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (state == ClientState::Connecting)
		{
			ImGui::Text("Connecting to server...");
		}
		else if (state == ClientState::Connected || state == ClientState::Game)
		{
			ImGui::Text("Connected to server");

			ImGui::Separator();

			ImGui::Text("Player info:");
			ImGui::Text(" - Id: %u", playerId);
			ImGui::Text(" - Name: %s", playerName.c_str());

			ImGui::Separator();

			ImGui::Text("Spaceship info:");
			ImGui::Text(" - Type: %u", spaceshipType);
			ImGui::Text(" - Network id: %u", networkId);

			vec2 playerPosition = {};
			GameObject *playerGameObject = App->modLinkingContext->getNetworkGameObject(networkId);
			if (playerGameObject != nullptr) {
				playerPosition = playerGameObject->position;
			}
			ImGui::Text(" - Coordinates: (%f, %f)", playerPosition.x, playerPosition.y);

			ImGui::Separator();

			ImGui::Text("Connection checking info:");
			ImGui::Text(" - Ping interval (s): %f", PING_INTERVAL_SECONDS);
			ImGui::Text(" - Disconnection timeout (s): %f", DISCONNECT_TIMEOUT_SECONDS);

			ImGui::Separator();

			ImGui::Text("Input:");
			ImGui::InputFloat("Delivery interval (s)", &inputDeliveryIntervalSeconds, 0.01f, 0.1f, 4);
		}
	}

	if (ImGui::Begin("Score Panel"))
	{
		if (state == ClientState::Connected)
		{
			if (ImGui::Button("Start"))
			{
				OutputMemoryStream packet;
				packet << PROTOCOL_ID;
				packet << ClientMessage::StartGame;

				sendPacket(packet, serverAddress);
			}
		}
		else if (state == ClientState::Game)
		{
			GameObject* playerGameObject = App->modLinkingContext->getNetworkGameObject(networkId);
			if (playerGameObject != nullptr) {
				Spaceship* spaceshipBehaviour = (Spaceship*)playerGameObject->behaviour;
				ImGui::Text("%s Score: %i", playerName.c_str(), spaceshipBehaviour->score);
			}
			
		}
		ImGui::End();
	}
}

void ModuleNetworkingClient::onPacketReceived(const InputMemoryStream &packet, const sockaddr_in &fromAddress)
{
	// TODO(you): UDP virtual connection lab session

	uint32 protoId;
	packet >> protoId;
	if (protoId != PROTOCOL_ID) return;

	ServerMessage message;
	packet >> message;

	if (state == ClientState::Connecting)
	{
		if (message == ServerMessage::Welcome)
		{
			packet >> playerId;
			packet >> networkId;

			LOG("ModuleNetworkingClient::onPacketReceived() - Welcome from server");
			state = ClientState::Connected;
		}
		else if (message == ServerMessage::Unwelcome)
		{
			WLOG("ModuleNetworkingClient::onPacketReceived() - Unwelcome from server :-(");
			disconnect();
		}
	}
	else if (state == ClientState::Connected ||state == ClientState::Game)
	{
		// TODO(you): World state replication lab session
		if (message == ServerMessage::Replication)
		{
			packet >> inputDataFront;
			if (deliveryManager.processSequenceNumber(packet))
			{
				repManagerClient.read(packet);

				GameObject* playerGameObject = App->modLinkingContext->getNetworkGameObject(networkId);
				if (playerGameObject == nullptr)
					return;
				
				// Client Side prediction
				InputController prevController;
				prevController = inputControllerFromInputPacketData(inputData[inputDataFront % ArrayCount(inputData)], prevController);

				for (int i = inputDataFront + 1; i < inputDataBack; ++i)
				{
					prevController = inputControllerFromInputPacketData(inputData[i % ArrayCount(inputData)], prevController);
					playerGameObject->behaviour->onInput(prevController);
				}

			}
		}
		else if (message == ServerMessage::LostHP)
		{
			packet >> currentHealth;
			
			if (currentHealth <= 0)
			{
				// Disconnect
			}
		}
		else if (message == ServerMessage::StartGame)
		{
			packet >> MAX_HEALTH;
			currentHealth = MAX_HEALTH;

			state = ClientState::Game;
		}
		
	}

	secondsSinceLastPackage = 0.0f;
}

void ModuleNetworkingClient::onUpdate()
{
	if (state == ClientState::Stopped) return;

	// TODO(you): UDP virtual connection lab session

	if (state == ClientState::Connecting)
	{
		secondsSinceLastPackage += Time.deltaTime;

		if (secondsSinceLastPackage > 0.1f)
		{
			secondsSinceLastPackage = 0.0f;

			OutputMemoryStream packet;
			packet << PROTOCOL_ID;
			packet << ClientMessage::Hello;
			packet << playerName;
			packet << spaceshipType;

			sendPacket(packet, serverAddress);
		}
	}
	else if (state == ClientState::Connected || state == ClientState::Game)
	{
		// Increae time to secondsSinceLastPackage
		secondsSinceLastPackage += Time.deltaTime;

		// TODO(you): Check if TIMEOUT greater than secondsSinceLastPackage
		if (secondsSinceLastPackage >= DISCONNECT_TIMEOUT_SECONDS)
		{
			WLOG("ModuleNetworkingClient::onUpdate() - TimeOut :-(");
			disconnect();
		}

		secondsSinceLastPing += Time.deltaTime;
		if (secondsSinceLastPing >= PING_INTERVAL_SECONDS)
		{
			OutputMemoryStream packet;
			packet << PROTOCOL_ID;
			packet << ClientMessage::Ping;
			deliveryManager.writeSequenceNumbersPendingAck(packet);
			sendPacket(packet, serverAddress);

			secondsSinceLastPing = 0.0f;
		}

		// Process more inputs if there's space
		if (inputDataBack - inputDataFront < ArrayCount(inputData))
		{
			// Pack current input
			uint32 currentInputData = inputDataBack++;
			InputPacketData &inputPacketData = inputData[currentInputData % ArrayCount(inputData)];
			inputPacketData.sequenceNumber = currentInputData;
			inputPacketData.horizontalAxis = Input.horizontalAxis;
			inputPacketData.verticalAxis = Input.verticalAxis;
			inputPacketData.buttonBits = packInputControllerButtons(Input);

			GameObject* playerGameObject = App->modLinkingContext->getNetworkGameObject(networkId);
			if (playerGameObject)
				playerGameObject->behaviour->onInput(Input);
		}

		secondsSinceLastInputDelivery += Time.deltaTime;

		// Input delivery interval timed out: create a new input packet
		if (secondsSinceLastInputDelivery > inputDeliveryIntervalSeconds)
		{
			secondsSinceLastInputDelivery = 0.0f;

			OutputMemoryStream packet;
			packet << PROTOCOL_ID;
			packet << ClientMessage::Input;

			// TODO(you): Reliability on top of UDP lab session

			for (uint32 i = inputDataFront; i < inputDataBack; ++i)
			{
				InputPacketData &inputPacketData = inputData[i % ArrayCount(inputData)];
				packet << inputPacketData.sequenceNumber;
				packet << inputPacketData.horizontalAxis;
				packet << inputPacketData.verticalAxis;
				packet << inputPacketData.buttonBits;
			}

			// Clear the queue
			//inputDataFront = inputDataBack;

			sendPacket(packet, serverAddress);
		}

		// TODO(you): Latency management lab session

		// Update camera for player
		//GameObject *playerGameObject = App->modLinkingContext->getNetworkGameObject(networkId);
		//if (playerGameObject != nullptr)
		//{
		//	App->modRender->cameraPosition = playerGameObject->position;
		//}
		//else
		//{
		//	// This means that the player has been destroyed (e.g. killed)
		//}

		// Interpolation of other objects
		uint16 objCount = 0;
		GameObject* networkGameObjects[MAX_NETWORK_OBJECTS] = {};
		App->modLinkingContext->getNetworkGameObjects(networkGameObjects, &objCount);

		for (int i = 0; i < objCount; ++i)
		{
			if (networkGameObjects[i]->networkId != networkId)
				networkGameObjects[i]->interpolate();
		}
	}
}

void ModuleNetworkingClient::onConnectionReset(const sockaddr_in & fromAddress)
{
	disconnect();
}

void ModuleNetworkingClient::onDisconnect()
{
	state = ClientState::Stopped;

	GameObject *networkGameObjects[MAX_NETWORK_OBJECTS] = {};
	uint16 networkGameObjectsCount;
	App->modLinkingContext->getNetworkGameObjects(networkGameObjects, &networkGameObjectsCount);
	App->modLinkingContext->clear();

	for (uint32 i = 0; i < networkGameObjectsCount; ++i)
	{
		Destroy(networkGameObjects[i]);
	}

	deliveryManager.clear();
	App->modRender->cameraPosition = {};
}
