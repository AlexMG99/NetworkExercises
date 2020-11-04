#include "ModuleNetworkingServer.h"


//////////////////////////////////////////////////////////////////////
// ModuleNetworkingServer public methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::start(int port)
{
	// TODO(jesus): TCP listen socket stuff
	// - Create the listenSocket
	// - Set address reuse
	// - Bind the socket to a local interface
	// - Enter in listen mode
	// - Add the listenSocket to the managed list of sockets using addSocket()

	listenSocket = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in bindAddr;
	bindAddr.sin_family = AF_INET;
	bindAddr.sin_port = htons(port);
	bindAddr.sin_addr.S_un.S_addr = INADDR_ANY;

	int result = bind(listenSocket, (const struct sockaddr*)&bindAddr, sizeof(bindAddr));

	if (result == SOCKET_ERROR)
		reportError("Socket not binded correctly the listen Socket ole");
	else
	{
		result = listen(listenSocket, 1);

		if (result == SOCKET_ERROR)
			reportError("Socket not listened correctly emoji llorar :(");
		else
		{
			addSocket(listenSocket);
			state = ServerState::Listening;
		}
	}

	return true;
}

bool ModuleNetworkingServer::isRunning() const
{
	return state != ServerState::Stopped;
}



//////////////////////////////////////////////////////////////////////
// Module virtual methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::update()
{
	return true;
}

bool ModuleNetworkingServer::gui()
{
	if (state != ServerState::Stopped)
	{
		// NOTE(jesus): You can put ImGui code here for debugging purposes
		ImGui::Begin("Server Window");

		Texture *tex = App->modResources->server;
		ImVec2 texSize(400.0f, 400.0f * tex->height / tex->width);
		ImGui::Image(tex->shaderResource, texSize);

		ImGui::Text("List of connected sockets:");

		for (auto &connectedSocket : connectedSockets)
		{
			ImGui::Separator();
			ImGui::Text("Socket ID: %d", connectedSocket.socket);
			ImGui::Text("Address: %d.%d.%d.%d:%d",
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b1,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b2,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b3,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b4,
				ntohs(connectedSocket.address.sin_port));
			ImGui::Text("Player name: %s", connectedSocket.playerName.c_str());
		}

		ImGui::End();
	}

	return true;
}



//////////////////////////////////////////////////////////////////////
// ModuleNetworking virtual methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::isListenSocket(SOCKET socket) const
{
	return socket == listenSocket;
}

void ModuleNetworkingServer::onSocketConnected(SOCKET socket, const sockaddr_in &socketAddress)
{
	// Add a new connected socket to the list
	ConnectedSocket connectedSocket;
	connectedSocket.socket = socket;
	connectedSocket.address = socketAddress;
	connectedSockets.push_back(connectedSocket);
}

void ModuleNetworkingServer::onSocketReceivedData(SOCKET socket, const InputMemoryStream& packet)
{
	ClientMessage clientMessage;
	packet >> clientMessage;

	// Set the player name of the corresponding connected socket proxy
	switch (clientMessage)
	{
	case ClientMessage::Hello:
		HandleHelloMessage(socket, packet);
		break;
	case ClientMessage::ChatMessage:
		break;
	default:
		break;
	}
}

void ModuleNetworkingServer::onSocketDisconnected(SOCKET socket)
{
	// Remove the connected socket from the list
	for (auto it = connectedSockets.begin(); it != connectedSockets.end(); ++it)
	{
		auto &connectedSocket = *it;
		if (connectedSocket.socket == socket)
		{
			connectedSockets.erase(it);
			break;
		}
	}
}

bool ModuleNetworkingServer::IsNameAvailable(const char* name)
{
	for (auto it = connectedSockets.begin(); it != connectedSockets.end(); ++it)
	{
		if (strcmp((*it).playerName.c_str(), name) == 0)
		{
			return false;
		}
	}

	return true;
}

void ModuleNetworkingServer::HandleHelloMessage(SOCKET s, const InputMemoryStream& packet)
{
	std::string playerName;
	packet >> playerName;

	for (auto& connectedSocket : connectedSockets)
	{
		if (connectedSocket.socket == s)
		{
			if (IsNameAvailable(playerName.c_str()))
			{
				connectedSocket.playerName = playerName;

				// Send message of connecting
				OutputMemoryStream message;
				message << ServerMessage::Welcome;
				message << "*****************************\n WELCOME TO THE CHAT\n Please type /help to see the aviable commands.\n *****************************";
				message << MessageType::Info;

				sendPacket(message, s);
			}
			else
			{
				// Send message of connecting
				OutputMemoryStream message;
				message << ServerMessage::NotWelcome;

				sendPacket(message, s);
			}

		}
	}
}
