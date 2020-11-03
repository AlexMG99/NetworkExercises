#include "ModuleNetworkingClient.h"


bool  ModuleNetworkingClient::start(const char * serverAddressStr, int serverPort, const char *pplayerName)
{
	playerName = pplayerName;
	playerName += '\0';

	// TODO(jesus): TCP connection stuff
	// - Create the socket
	// - Create the remote address object
	// - Connect to the remote address
	// - Add the created socket to the managed list of sockets using addSocket()
	// - If everything was ok... change the state

	clientSocket = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in remoteAddr;
	remoteAddr.sin_family = AF_INET;
	remoteAddr.sin_port = htons(serverPort);
	inet_pton(AF_INET, serverAddressStr, &remoteAddr.sin_addr);

	int len = sizeof(remoteAddr);
	int result = connect(clientSocket, (sockaddr*)&remoteAddr, len);

	if (result == SOCKET_ERROR)
		reportError("Socket not connected correctly :_)");
	else
	{
		addSocket(clientSocket);
		state = ClientState::Start;
	}

	return true;
}

bool ModuleNetworkingClient::isRunning() const
{
	return state != ClientState::Stopped;
}

bool ModuleNetworkingClient::update()
{
	if (state == ClientState::Start)
	{
		OutputMemoryStream packet;
		packet << ClientMessage::Hello;
		packet << playerName;

		if (sendPacket(packet, clientSocket))
		{
			state = ClientState::Logging;
		}
		else
		{
			disconnect();
			state = ClientState::Stopped;
		}
	}

	return true;
}

bool ModuleNetworkingClient::gui()
{
	if (state != ClientState::Stopped)
	{
		// NOTE(jesus): You can put ImGui code here for debugging purposes
		ImGui::Begin("Client Window");

		Texture *tex = App->modResources->client;
		ImVec2 texSize(400.0f, 400.0f * tex->height / tex->width);
		ImGui::Image(tex->shaderResource, texSize);
		ImGui::Text("%s connected to the server...", playerName.c_str());

		for (auto line : fuckingChat)
		{
			switch (line.first)
			{
			case MessageType::Default:
				ImGui::Text("%s", line.second.c_str());
				break;
			case MessageType::Info:
				ImGui::TextColored(ImVec4(255, 255, 0, 255), "%s", line.second.c_str());
				break;
			default:
				break;
			}
			
		}

		ImGui::End();
	}

	return true;
}

void ModuleNetworkingClient::onSocketReceivedData(SOCKET socket, const InputMemoryStream& packet)
{
	ClientMessage clientMessage;
	packet >> clientMessage;

	// Set the player name of the corresponding connected socket proxy
	if (clientMessage == ClientMessage::Hello)
	{
		packet >> playerName;
	}
	else if (clientMessage == ClientMessage::Welcome)
	{
		std::string welcomeMessage;
		packet >> welcomeMessage;
		MessageType type;
		packet >> type;
		fuckingChat.insert(std::pair<MessageType,std::string>(type, welcomeMessage));
	}
}

void ModuleNetworkingClient::onSocketDisconnected(SOCKET socket)
{
	state = ClientState::Stopped;
}

