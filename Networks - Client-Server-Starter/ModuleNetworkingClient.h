#pragma once

#include "ModuleNetworking.h"

//TODO: Remove includes
#include "imgui/imgui.h"
#include <map>

class ModuleNetworkingClient : public ModuleNetworking
{
public:

	//////////////////////////////////////////////////////////////////////
	// ModuleNetworkingClient public methods
	//////////////////////////////////////////////////////////////////////

	bool start(const char *serverAddress, int serverPort, const char *playerName);

	bool isRunning() const;



private:

	//////////////////////////////////////////////////////////////////////
	// Module virtual methods
	//////////////////////////////////////////////////////////////////////

	bool update() override;

	bool gui() override;



	//////////////////////////////////////////////////////////////////////
	// ModuleNetworking virtual methods
	//////////////////////////////////////////////////////////////////////

	void onSocketReceivedData(SOCKET socket, const InputMemoryStream& packet) override;

	void onSocketDisconnected(SOCKET socket) override;

	void HandleServerMessage(SOCKET socket, const InputMemoryStream& packet);
	void HandleNotWelcomeMessage(SOCKET socket, const InputMemoryStream& packet);



	//////////////////////////////////////////////////////////////////////
	// Client state
	//////////////////////////////////////////////////////////////////////

	enum class ClientState
	{
		Stopped,
		Start,
		Logging,
		Disconnected,
		Logged
	};

	ClientState state = ClientState::Stopped;

	sockaddr_in serverAddress = {};
	SOCKET clientSocket = INVALID_SOCKET;

	std::string playerName;
	ImVec4 colorName = ImVec4(255, 255, 255, 255);

	struct ChatText {
		ChatText(std::string tM, MessageType mT) { textMessage = tM; mType = mT; };
		std::string textMessage;
		MessageType mType;
	};

	std::vector<ChatText> fuckingChat;

};

