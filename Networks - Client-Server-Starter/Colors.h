#pragma once

#include "imgui/imgui.h"

#define MAX_COLORS 7

ImVec4 White	= ImVec4(255, 255, 255, 255);
ImVec4 Blue		= ImVec4(0, 0, 255, 255);
ImVec4 Red		= ImVec4(255, 0, 0, 255);
ImVec4 FirebrickRed = ImVec4(178, 34, 34, 255);
ImVec4 Green	= ImVec4(0, 255, 0, 255);
ImVec4 LimeGreen = ImVec4(50, 205, 50, 255);
ImVec4 Yellow	= ImVec4(255, 255, 0, 255);
ImVec4 Orange	= ImVec4(255, 165, 0, 255);
ImVec4 Purple	= ImVec4(160, 32, 240, 255);
ImVec4 Pink		= ImVec4(255, 192, 0, 255);

ImVec4 colors[MAX_COLORS] = { Blue, Red, Green, Yellow, Orange, Purple, Pink };