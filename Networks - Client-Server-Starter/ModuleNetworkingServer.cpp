#include "ModuleNetworkingServer.h"
#include <random>


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
		HandleChatMessage(socket, packet);
		break;
	case ClientMessage::Disconnection:
		HandleUserDisconnectionMessage(socket, packet);
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
	int colPos;
	packet >> colPos;

	for (auto& connectedSocket : connectedSockets)
	{
		if (connectedSocket.socket == s)
		{
			OutputMemoryStream message;
			if (IsNameAvailable(playerName.c_str()))
			{
				connectedSocket.playerName = playerName;
				connectedSocket.colorPosition = colPos;

				// Send message of connecting
				message << ServerMessage::Welcome;
				message << "******************************************************\n              WELCOME TO THE CHAT\n Please type /help to see the aviable commands.\n******************************************************";
				message << MessageType::Info;
				sendPacket(message, s);

				HandleUserConnectionMessage(s, connectedSocket.playerName.c_str());
			}
			else
			{
				// Send message of connecting
				message << ServerMessage::NotWelcome;
				sendPacket(message, s);
			}
			
		}
	}
}

void ModuleNetworkingServer::HandleChatMessage(SOCKET s, const InputMemoryStream& packet)
{
	// Recieve message information
	std::string playerName;
	packet >> playerName;
	std::string message;
	packet >> message;
	int cPos;
	packet >> cPos;

	//Check if its a command or a message
	OutputMemoryStream chatMessage;

	if (message[0] == '/')
	{
		std::string firstCommand;
		std::string afterCommand;
		int pos = message.find(' ');
		if(pos == -1)
			firstCommand = message.substr(1, pos);
		else
		{
			firstCommand = message.substr(1, pos - 1);
			afterCommand = message.substr(pos + 1, message.length());
		}

		// Commands
		if (strcmp(firstCommand.c_str(), "help") == 0)
		{
			chatMessage << ServerMessage::Command;
			chatMessage << "******************************************************\nThe commands you can use are:\n-/list: show list of all users.\n-/kick: to disconnect some other user from the chat.\n-/whisper: to send a message only to one user.\n-/change_name: to change your username.\n-/p help: shows pokebot commands\n******************************************************";
			chatMessage << MessageType::Help;
		}
		else if (strcmp(firstCommand.c_str(), "list") == 0)
		{
			std::string userNames = "******************************************************\nList of users:\n";
			for (auto& connectedSocket : connectedSockets)
			{
				userNames += connectedSocket.playerName + "\n";
			}
			userNames += "******************************************************";

			chatMessage << ServerMessage::Command;
			chatMessage << userNames;
			chatMessage << MessageType::List;
		}
		else if (strcmp(firstCommand.c_str(), "p") == 0)
		{
#pragma region Pokebot
			std::string pokeCommand;
			std::string afterPokeCommand;
			int pokePos = afterCommand.find(' ');
			pokeCommand = afterCommand.substr(0, pokePos);

			if (strcmp(pokeCommand.c_str(), "help") == 0)
			{
				chatMessage << ServerMessage::Command;
				chatMessage << "******************************************************\nThe pokemon commands you can use are:\n-/p on: turns on the bot.\n-/p off: turns off the bot.\n-/p catch name: catch the last pokemon if named is guessed and you are the first one.\n-/p team: show your pokemon list.\n-/p change_rand: change random percentage from 5 to 50\n******************************************************";
				chatMessage << MessageType::Help;
			}
			else if (strcmp(pokeCommand.c_str(), "on") == 0)
			{
				pokebot.turnedOn = true;
				chatMessage << ServerMessage::Command;
				chatMessage << "Pokebot turned on. Go catch'em all!";
				chatMessage << MessageType::Pokemon;
				srand(time(NULL));

				for (auto& connectedSocket : connectedSockets)
				{
					if(connectedSocket.socket != s)
						sendPacket(chatMessage, connectedSocket.socket);
				}
			}
			else if (strcmp(pokeCommand.c_str(), "off") == 0)
			{
				pokebot.turnedOn = false;
				chatMessage << ServerMessage::Command;
				chatMessage << "Pokebot turned off. Goodbye little Pichu owo!";
				chatMessage << MessageType::Pokemon;

				for (auto& connectedSocket : connectedSockets)
				{
					if (connectedSocket.socket != s)
						sendPacket(chatMessage, connectedSocket.socket);
				}
			}
			else if (strcmp(pokeCommand.c_str(), "catch") == 0)
			{
				int pokePos2 = afterCommand.find(' ');
				std::string pokeName;
				if (pokePos != -1)
				{
					if (pokebot.lastPokemon.pokedexNum == -1)
					{
						chatMessage << ServerMessage::Command;
						chatMessage << "There's no Pokemon in the tall grass...";
						chatMessage << MessageType::Pokemon;
					}
					else
					{
						pokeName = afterCommand.substr(pokePos2 + 1, afterCommand.length());

						if (strcmp(pokebot.lastPokemon.name.c_str(), pokeName.c_str()) == 0)
						{
							chatMessage << ServerMessage::PokeCatch;
							chatMessage << "Congratulations! " + pokeName + " has been added to your poke team!";
							chatMessage << MessageType::Pokemon;
							chatMessage << (pokebot.lastPokemon.pokedexNum - 1);
							chatMessage << (pokebot.lastPokemon.name);

							OutputMemoryStream pityMessage;
							pityMessage << ServerMessage::Command;
							pityMessage << playerName + " has catched " + pokeName + ". Better luck next time ;)";
							pityMessage << MessageType::Pokemon;

							for (auto& connectedSocket : connectedSockets)
							{
								if(connectedSocket.socket != s)
									sendPacket(pityMessage, connectedSocket.socket);
								else
									connectedSocket.pokemons.push_back(Pokemon(pokebot.lastPokemon.pokedexNum, pokebot.lastPokemon.name.c_str()));
							}

							pokebot.lastPokemon = Pokemon();
						}
						else
						{
							chatMessage << ServerMessage::Command;
							chatMessage << "You really wrote " + pokeName + "? Are you pikablind or just dislexic?";
							chatMessage << MessageType::Pokemon;
						}
					}
				}
				else
				{
					chatMessage << ServerMessage::Command;
					chatMessage << "You have to write a pokemon name, Ditto head!";
					chatMessage << MessageType::Pokemon;
				}

				
			}
			else if (strcmp(pokeCommand.c_str(), "team") == 0)
			{
				std::string pokeTeam = "******************************************************\nList of pokemons:\n";
				for (auto& connectedSocket : connectedSockets)
				{
					if (connectedSocket.socket == s)
					{
						for (auto pokemon : connectedSocket.pokemons)
						{
							pokeTeam += pokemon.name + "\n";
						}
					}
				}
				pokeTeam += "******************************************************";

				chatMessage << ServerMessage::Command;
				chatMessage << pokeTeam;
				chatMessage << MessageType::Pokemon;
			}
			else if (strcmp(pokeCommand.c_str(), "change_rand") == 0)
			{
			int pokePos2 = afterCommand.find(' ');
			std::string pokeNum = afterCommand.substr(pokePos2 + 1, afterCommand.length());;
			if (pokePos != -1)
			{
				if (isNumber(pokeNum))
				{
					int num = std::stoi(pokeNum);
					if (5 <= num && num <= 50)
					{
						pokebot.randPercetage = num;
						chatMessage << ServerMessage::Command;
						chatMessage << "Rand percentage changed to " + pokeNum;
						chatMessage << MessageType::Pokemon;

						for (auto& connectedSocket : connectedSockets)
						{
							if (connectedSocket.socket != s)
							{
								sendPacket(chatMessage, connectedSocket.socket);
							}
						}
					}
					else
					{
						chatMessage << ServerMessage::Command;
						chatMessage << "Number has to be between 5 and 50.";
						chatMessage << MessageType::Pokemon;
					}
				}
				else
				{
					chatMessage << ServerMessage::Command;
					chatMessage << "You did not write a number";
					chatMessage << MessageType::Pokemon;
				}
				
			}
			else
			{
				chatMessage << ServerMessage::Command;
				chatMessage << "You have to write a number, Ditto head!";
				chatMessage << MessageType::Pokemon;
			}
			}
			else
			{
				chatMessage << ServerMessage::Command;
				chatMessage << "Wrong pokecommand! If you need help write /p help";
				chatMessage << MessageType::Error;
			}

#pragma endregion
		}
		else // Two or more words commands
		{
			int pos2 = afterCommand.find_first_of(" ");

			if (!afterCommand.empty())
			{
				std::string name = afterCommand.substr(0, pos2);

				// Change Name
				if (strcmp(firstCommand.c_str(), "change_name") == 0)
				{
					if (IsNameAvailable(name.c_str()))
					{
						chatMessage << ServerMessage::ChangeName;
						chatMessage << name;
						chatMessage << MessageType::Info;

						for (auto& connectedSocket : connectedSockets)
						{
							if (connectedSocket.socket == s)
								connectedSocket.playerName = name;
						}
					}
					else
					{
						chatMessage << ServerMessage::Command;
						chatMessage << "Username already taken. Please try a new one";
						chatMessage << MessageType::Error;
					}
					
				}
				else if (strcmp(firstCommand.c_str(), "kick") == 0)
				{
					OutputMemoryStream kickMessage;
					bool userExist = false;
					kickMessage << ServerMessage::Kick;
					kickMessage << "You have been kicked by " + playerName;

					for (auto& connectedSocket : connectedSockets)
					{
						if (strcmp(connectedSocket.playerName.c_str(), name.c_str()) == 0)
						{
							sendPacket(kickMessage, connectedSocket.socket);
							userExist = true;
						}
					}

					if (!userExist)
					{
						chatMessage << ServerMessage::Command;
						chatMessage << "User not founded. Unable to kick";
						chatMessage << MessageType::Error;
					}
					else
						return;
				}
				else if (strcmp(firstCommand.c_str(), "whisper") == 0)
				{
					OutputMemoryStream whisper;
					bool userExist = false;
					std::string whisperMessage = afterCommand.substr(pos2 + 1, afterCommand.length());

					whisper << ServerMessage::WhisperMessage;
					whisper << playerName;
					whisper << whisperMessage;
					whisper << MessageType::Whisper;

					for (auto& connectedSocket : connectedSockets)
					{
						if (strcmp(connectedSocket.playerName.c_str(), name.c_str()) == 0)
						{
							sendPacket(whisper, connectedSocket.socket);
							userExist = true;
						}
					}

					if (!userExist)
					{
						chatMessage << ServerMessage::Command;
						chatMessage << "User not founded. Unable to whisper";
						chatMessage << MessageType::Error;
					}
					else
						return;
				}
			}
			else
			{
				chatMessage << ServerMessage::Command;
				chatMessage << "Wrong command! If you need help write /help";
				chatMessage << MessageType::Error;
			}
			
		}

		sendPacket(chatMessage, s);
	}
	else
	{
#pragma region pokeSpawn
	if (pokebot.turnedOn && (rand() % 100) < pokebot.randPercetage)
	{
		pokebot.lastPokemon = pokebot.pokemons[rand() % 150];
		chatMessage << ServerMessage::PokeSpawn;
		chatMessage << pokebot.lastPokemon.name + " has spawned! Type /p catch <pokemon_name> to catch it!";
		chatMessage << MessageType::Pokemon;
	}
	else
	{
		chatMessage << ServerMessage::UserMessage;
	}
#pragma endregion
		chatMessage << playerName;
		chatMessage << message;
		chatMessage << MessageType::Message;
		chatMessage << cPos;

		for (auto& connectedSocket : connectedSockets)
		{
			sendPacket(chatMessage, connectedSocket.socket);
		}
	}
}

void ModuleNetworkingServer::HandleUserConnectionMessage(SOCKET s, const char* pName)
{
	// Server debug information
	LOG("User %s connected to the server", pName);

	// Notice other users that one user has been disconnected
	OutputMemoryStream conMessage;
	conMessage << ServerMessage::UserConnection;
	conMessage << pName;
	conMessage << MessageType::Connection;

	for (auto& connectedSocket : connectedSockets)
	{
		if(connectedSocket.socket != s)
			sendPacket(conMessage, connectedSocket.socket);
	}
}

void ModuleNetworkingServer::HandleUserDisconnectionMessage(SOCKET s, const InputMemoryStream& packet)
{
	// Server debug information
	std::string playerName;
	packet >> playerName;

	LOG("User %s disconnected from the server", playerName.c_str());

	// Notice other users that one user has been disconnected
	OutputMemoryStream message;
	message << ServerMessage::UserDisconnection;
	message << playerName;
	message << MessageType::Disconnection;

	for (auto& connectedSocket : connectedSockets)
	{
		if (connectedSocket.socket != s)
			sendPacket(message, connectedSocket.socket);
	}

}

bool ModuleNetworkingServer::isNumber(std::string s)
{
	for (int i = 0; i < s.length(); i++)
		if (isdigit(s[i]) == false)
			return false;

	return true;
}
