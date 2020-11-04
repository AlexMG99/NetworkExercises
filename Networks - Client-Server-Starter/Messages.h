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
	UserConnection,
	UserMessage,
	UserDisconnection
};

enum class MessageType
{
	Default,
	Connection,
	Disconnection,
	Info,
	Message,
	Warning,
	Error
};

