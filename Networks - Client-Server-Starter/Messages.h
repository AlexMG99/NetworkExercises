#pragma once

// Add as many messages as you need depending on the
// functionalities that you decide to implement.

enum class ClientMessage
{
	Hello,
	ChatMessage,
	Connection,
	Disconnection
};

enum class ServerMessage
{
	Welcome,
	NotWelcome,
	Command,
	ChangeName,
	Kick,
	UserConnection,
	UserMessage,
	WhisperMessage,
	UserDisconnection,
	PokeSpawn,
	PokeCatch
};

enum class MessageType
{
	Default,
	Connection,
	Disconnection,
	Help,
	List,
	Info,
	Pokemon,
	Whisper,
	Message,
	Warning,
	Error
};

