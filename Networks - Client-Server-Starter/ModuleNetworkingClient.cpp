#include "Colors.h"
#include "ModuleNetworkingClient.h"
#include <random>

#define MAX_CHAR 256

bool  ModuleNetworkingClient::start(const char * serverAddressStr, int serverPort, const char *pplayerName)
{
	playerName = pplayerName;

	// User color
	srand(time(NULL));
	colorPos = rand() % (MAX_COLORS - 1);

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
		packet << colorPos;

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
		OutputMemoryStream message;
		message << ClientMessage::Disconnection;
		message << playerName;

		sendPacket(message, clientSocket);

		LOG("Disconnected to the server succesfully");
		disconnect();
		state = ClientState::Stopped;
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
		for (auto line = fuckingChat.begin(); line != fuckingChat.end(); ++line)
		{
			switch ((*line).mType)
			{
			case MessageType::Default:
				ImGui::Text("%s", (*line).textMessage.c_str());
				break;
			case MessageType::Info:
				ImGui::TextColored(Yellow, "%s", (*line).textMessage.c_str());
				break;
			case MessageType::Message:
				ImGui::TextColored(colors[(*line).colorPos], "%s: ", (*line).pName.c_str()); ImGui::SameLine();
				ImGui::TextColored(White, "%s", (*line).textMessage.c_str());
				break;
			case MessageType::Help:
				ImGui::TextColored(Pink, "%s", (*line).textMessage.c_str());
				break;
			case MessageType::List:
				ImGui::TextColored(DodgeBlue, "%s", (*line).textMessage.c_str());
				break;
			case MessageType::Connection:
				ImGui::TextColored(LimeGreen, "         ****** %s joined ******", (*line).textMessage.c_str());
				break;
			case MessageType::Disconnection:
				ImGui::TextColored(FirebrickRed, "         ****** %s left ******", (*line).textMessage.c_str());
			break;
			case MessageType::Error:
				ImGui::TextColored(Red, "%s", (*line).textMessage.c_str());
				break;
			case MessageType::Whisper:
				ImGui::TextColored(Purple, "%s", (*line).textMessage.c_str());
				break;
			case MessageType::Pokemon:
				ImGui::TextColored(PokeColor, "%s", (*line).textMessage.c_str());
				break;
			default:
				break;
			}
			
		}

		ImGui::EndChild();

		static char chatText[MAX_CHAR];
		if (ImGui::InputText("Chat", chatText, IM_ARRAYSIZE(chatText), ImGuiInputTextFlags_EnterReturnsTrue))
		{
			OutputMemoryStream message;
			message << ClientMessage::ChatMessage;
			message << playerName;
			message << chatText;
			message << colorPos;
			
			sendPacket(message, clientSocket);

			memset(chatText, 0, MAX_CHAR);

			//Continue focusing chat
			ImGui::SetKeyboardFocusHere();
		}

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
		HandleServerMessage(socket, packet);
		break;
	case ServerMessage::NotWelcome:
		HandleNotWelcomeMessage(socket, packet);
		break;
	case ServerMessage::Command:
		HandleServerMessage(socket, packet);
		break;
	case ServerMessage::ChangeName:
		HandleChangeNameMessage(socket, packet);
		break;
	case ServerMessage::Kick:
		HandleKickMessage(socket, packet);
		break;
	case ServerMessage::UserConnection:
		HandleServerMessage(socket, packet);
		break;
	case ServerMessage::UserMessage:
		HandleChatMessage(socket, packet);
		break;
	case ServerMessage::WhisperMessage:
		HandleWhisperMessage(socket, packet);
		break;
	case ServerMessage::UserDisconnection:
		HandleServerMessage(socket, packet);
		break;
	case ServerMessage::PokeSpawn:
		HandlePokemonSpawnMessage(socket, packet);
		break;
	case ServerMessage::PokeCatch:
		HandlePokemonCatchMessage(socket, packet);
		break;
	default:
		break;
	}
}

void ModuleNetworkingClient::onSocketDisconnected(SOCKET socket)
{
	state = ClientState::Stopped;
}

void ModuleNetworkingClient::HandleServerMessage(SOCKET socket, const InputMemoryStream& packet)
{
	std::string message;
	packet >> message;
	MessageType type;
	packet >> type;

	ChatText chatText = ChatText(message, type);
	fuckingChat.push_back(chatText);
	
}

void ModuleNetworkingClient::HandlePokemonSpawnMessage(SOCKET socket, const InputMemoryStream& packet)
{
	std::string pokeMessage;
	packet >> pokeMessage;
	MessageType type;
	packet >> type;

	HandleChatMessage(socket, packet);

	ChatText chatText = ChatText(pokeMessage, type);
	fuckingChat.push_back(chatText);
}

void ModuleNetworkingClient::HandlePokemonCatchMessage(SOCKET socket, const InputMemoryStream& packet)
{
	HandleServerMessage(socket, packet);

	int lisPos;
	packet >> lisPos;
	std::string pokeName;
	packet >> pokeName;

	pokeTeam.push_back(Pokemon(lisPos, pokeName.c_str()));

}

void ModuleNetworkingClient::HandleChangeNameMessage(SOCKET socket, const InputMemoryStream& packet)
{
	std::string name;
	packet >> name;
	playerName = name;
	MessageType type;
	packet >> type;

	std::string message = "Name changed to " + name;

	ChatText chatText = ChatText(message, type);
	fuckingChat.push_back(chatText);
}

void ModuleNetworkingClient::HandleKickMessage(SOCKET socket, const InputMemoryStream& packet)
{
	std::string message;
	packet >> message;

	WLOG("%s", message.c_str());

	state = ClientState::Disconnected;
}

void ModuleNetworkingClient::HandleChatMessage(SOCKET socket, const InputMemoryStream& packet)
{
	std::string pName;
	packet >> pName;
	std::string message;
	packet >> message;
	MessageType type;
	packet >> type;
	int cPos;
	packet >> cPos;

	ChatText chatText = ChatText(message, type, pName, cPos);
	fuckingChat.push_back(chatText);
}

void ModuleNetworkingClient::HandleWhisperMessage(SOCKET socket, const InputMemoryStream& packet)
{
	std::string pName;
	packet >> pName;
	std::string message;
	packet >> message;
	MessageType type;
	packet >> type;

	std::string finalMessage = pName + " whispers you: " + message;

	ChatText chatText = ChatText(finalMessage, type, pName);
	fuckingChat.push_back(chatText);
}

void ModuleNetworkingClient::HandleNotWelcomeMessage(SOCKET socket, const InputMemoryStream& packet)
{
	ELOG("Sorry, username %s already taken. Please try a new one :3", playerName.c_str());
	state = ClientState::Disconnected;
}


