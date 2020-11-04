#include "Colors.h"
#include "ModuleNetworkingClient.h"
#include <random>

bool  ModuleNetworkingClient::start(const char * serverAddressStr, int serverPort, const char *pplayerName)
{
	playerName = pplayerName;
	playerName += '\0';

	//TODO: Check colors
	//colorName = colors[(rand() % (MAX_COLORS - 1))];

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
	switch (state)
	{
	case ClientState::Start:
	{
		OutputMemoryStream packet;
		packet << ClientMessage::Hello;
		packet << playerName;

		if (sendPacket(packet, clientSocket))
			state = ClientState::Logging;
		else
		{
			disconnect();
			state = ClientState::Stopped;
		}
	}
		break;
	case ClientState::Logging:
		LOG("Connected to the server succesfully");
		state = ClientState::Logged;
		break;
	case ClientState::Disconnected:
	{
		LOG("Disconnected to the server succesfully");
		disconnect();
		state = ClientState::Stopped;
		//TODO: send message to server
	}
		break;
	default:
		break;
	}

	return true;
}

bool ModuleNetworkingClient::gui()
{
	if (state != ClientState::Stopped)
	{
		// NOTE(jesus): You can put ImGui code here for debugging purposes
		ImGui::ShowDemoWindow();
		ImGui::Begin("Client Window");

		Texture *tex = App->modResources->client;
		ImVec2 texSize(400.0f, 400.0f * tex->height / tex->width);
		ImGui::Image(tex->shaderResource, texSize);
		ImGui::Text("Hello %s. Welcome to the server", playerName.c_str()); ImGui::SameLine();
		if (ImGui::Button("Logout"))
		{
			state = ClientState::Disconnected;
		}

		ImGui::BeginChild("##ChatBox", ImVec2(ImGui::GetWindowContentRegionWidth(), ImGui::GetContentRegionAvail().y), true);
		for (auto line : fuckingChat)
		{
			switch (line.first)
			{
			case MessageType::Default:
				ImGui::Text("%s", line.second.c_str());
				break;
			case MessageType::Info:
				ImGui::TextColored(Yellow, "%s", line.second.c_str());
				break;
			case MessageType::Message:
				ImGui::TextColored(colorName, "%s: ", playerName.c_str()); ImGui::SameLine();
				ImGui::TextColored(White, "%s", line.second.c_str());
				break;
			default:
				break;
			}
			
		}

		ImGui::EndChild();

		ImGui::End();
	}

	return true;
}

void ModuleNetworkingClient::onSocketReceivedData(SOCKET socket, const InputMemoryStream& packet)
{
	ServerMessage serverMessage;
	packet >> serverMessage;

	// Set the player name of the corresponding connected socket proxy
	switch (serverMessage)
	{
	case ServerMessage::Welcome:
		HandleWelcomeMessage(socket, packet);
		break;
	case ServerMessage::NotWelcome:
		HandleNotWelcomeMessage(socket, packet);
		break;
	default:
		break;
	}
}

void ModuleNetworkingClient::onSocketDisconnected(SOCKET socket)
{
	state = ClientState::Stopped;
}

void ModuleNetworkingClient::HandleWelcomeMessage(SOCKET socket, const InputMemoryStream& packet)
{
	std::string welcomeMessage;
	packet >> welcomeMessage;
	MessageType type;
	packet >> type;
	fuckingChat.insert(std::pair<MessageType, std::string>(type, welcomeMessage));
}

void ModuleNetworkingClient::HandleNotWelcomeMessage(SOCKET socket, const InputMemoryStream& packet)
{
	ELOG("Sorry, username %s already taken. Please try a new one :3", playerName.c_str());
	state = ClientState::Disconnected;
}

