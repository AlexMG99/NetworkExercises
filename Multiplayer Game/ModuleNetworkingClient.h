#pragma once

#include "ModuleNetworking.h"

class ModuleNetworkingClient : public ModuleNetworking
{
public:

	//////////////////////////////////////////////////////////////////////
	// ModuleNetworkingClient public methods
	//////////////////////////////////////////////////////////////////////

	void setServerAddress(const char *serverAddress, uint16 serverPort);

	void setPlayerInfo(const char *playerName, uint8 spaceshipType);

	uint32 GetNetworkId() { return networkId; }

	sockaddr_in GetServerAdress() { return serverAddress; }


private:

	//////////////////////////////////////////////////////////////////////
	// ModuleNetworking virtual methods
	//////////////////////////////////////////////////////////////////////

	bool isClient() const override { return true; }

	void onStart() override;

	void onGui() override;

	void onPacketReceived(const InputMemoryStream &packet, const sockaddr_in &fromAddress) override;

	void onUpdate() override;

	void onConnectionReset(const sockaddr_in &fromAddress) override;

	void onDisconnect() override;


	//////////////////////////////////////////////////////////////////////
	// Client state
	//////////////////////////////////////////////////////////////////////

	enum class ClientState
	{
		Stopped,
		Connecting,
		Connected,
		Game
	};

	ClientState state = ClientState::Stopped;

	std::string serverAddressStr;
	uint16 serverPort = 0;

	sockaddr_in serverAddress = {};
	std::string playerName = "player";
	uint8 spaceshipType = 0;

	uint32 playerId = 0;
	uint32 networkId = 0;

	int playerScore = 0;
	int totalScore = 0;

	int currentHealth = 0;
	int MAX_HEALTH = 0;

	// Connecting stage

	float secondsSinceLastPackage = 0.0f;
	float secondsSinceLastPing = 0.0f;

	bool isDead = false;


	// Input ///////////

	static const int MAX_INPUT_DATA_SIMULTANEOUS_PACKETS = 64;

	InputPacketData inputData[MAX_INPUT_DATA_SIMULTANEOUS_PACKETS];
	uint32 inputDataFront = 0;
	uint32 inputDataBack = 0;

	float inputDeliveryIntervalSeconds = 0.05f;
	float secondsSinceLastInputDelivery = 0.0f;



	//////////////////////////////////////////////////////////////////////
	// Virtual connection
	//////////////////////////////////////////////////////////////////////

	// TODO(you): UDP virtual connection lab session

	DeliveryManager deliveryManager;

	//////////////////////////////////////////////////////////////////////
	// Replication
	//////////////////////////////////////////////////////////////////////

	// TODO(you): World state replication lab session
	ReplicationManagerClient repManagerClient;


	//////////////////////////////////////////////////////////////////////
	// Delivery manager
	//////////////////////////////////////////////////////////////////////

	// TODO(you): Reliability on top of UDP lab session



	//////////////////////////////////////////////////////////////////////
	// Latency management
	//////////////////////////////////////////////////////////////////////

	// TODO(you): Latency management lab session

};

