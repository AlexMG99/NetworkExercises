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
	void HandleChangeNameMessage(SOCKET socket, const InputMemoryStream& packet);
	void HandleKickMessage(SOCKET socket, const InputMemoryStream& packet);
	void HandleChatMessage(SOCKET socket, const InputMemoryStream& packet);
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
	int colorPos = 0;

	struct ChatText {
		ChatText(std::string tM, MessageType mT, std::string pN = "", int cP = 0) { textMessage = tM; mType = mT; pName = pN; colorPos = cP; };
		std::string textMessage = "";
		MessageType mType;
		std::string pName = "";
		int colorPos = 0;
	};

	std::vector<ChatText> fuckingChat;

};

