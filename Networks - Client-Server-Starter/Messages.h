#pragma once

// Add as many messages as you need depending on the
// functionalities that you decide to implement.

enum class ClientMessage
{
	Hello,
	ChatMessage
};

enum class ServerMessage
{
	Welcome,
	NotWelcome
};

enum class MessageType
{
	Default,
	Info,
	Message,
	Warning,
	Error
};

