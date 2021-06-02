#include "Xbox360Controller.h"
#include <iostream>
Xbox360Controller::Xbox360Controller()
{

	// int car = controllerState.Gamepad.bLeftTrigger;

}

Xbox360Controller::~Xbox360Controller()
{
}

void Xbox360Controller::update()
{
	/*int car = 4;
	if( XInputGetState(XINPUT_GAMEPAD_A, controllerState) == true);
	std::cout << car << std::endl;*/
}

bool Xbox360Controller::isConnected()
{
	return false;
}

bool Xbox360Controller::connect()
{
	return false;
}
