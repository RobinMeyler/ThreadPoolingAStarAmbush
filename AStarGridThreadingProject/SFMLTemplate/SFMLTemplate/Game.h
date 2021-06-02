// Author: Robin Meyler
// Date: 2021
// Title: Thread Pool Implmentation of A* Ambush

#ifndef GAME
#define GAME

#include <SFML\Graphics.hpp>
#include <vector>
#include"Xbox360Controller.h"
#include "ThreadPool.h"
#include <ctime>
#include <thread>
#include <string>
#include <iostream>
#include <future>
#include "MyVector3.h"
#include "Globals.h"

class Game
{
public:
	Game();
	~Game();
	void run();

	// Astar function used in the threadpool
	void aStar(Node* t_start, Node* t_end, std::vector<Node*>* path, int index);
	// Tells the program when an agent has finished it's Astar on a thread
	void finished(int t_index);

	// Setting up the map
	void setupSmall();
	void setupMedium(int t_numberOfAgents);
	void setupLarge();

	// Public Variables
	bool doRetro{ false };
	int gridSize{ 1000 };
	int gridSizeTotal = gridSize * gridSize;
	int numOfAgents{ 50 };

private:
	void processEvents();				
	void update(sf::Time t_deltaTime);
	void render();
	void rightClickScroll(MyVector3 mouseClick);
	void reset();

	// Threaing variables
	ThreadPool threadpool;
	std::mutex ambushLock;
	std::mutex finishedLock;
	std::mutex pathLock;
	std::mutex nodesLock;
	std::condition_variable waitForLock;

	// Program data
	std::vector<Node*> m_gameNodes;
	std::vector<sf::RectangleShape*> m_tiles;
	std::vector<std::vector<Node*>*> paths;
	std::vector<int> skipCounter;
	std::vector<Node*> starts;
	std::vector<sf::RectangleShape*> m_agents;
	std::vector<int> m_finishedIds;
	std::vector<int> m_nexts;

	// Goal/Player
	sf::RectangleShape m_goalSquare;
	Node* m_goal;

	// Time management
	sf::Time fpsDelay;
	sf::Time m_moveTimer;
	sf::Time m_clickTimer;
	sf::Clock fpsClock;
	sf::Clock timeManager;
	sf::Clock timeManager2;

	// Display
	sf::Text m_fps;
	sf::Font m_font;
	sf::RenderWindow m_window; // main SFML window
	sf::View m_camera;
	sf::RenderTexture m_gameMap;

	sf::Event event;

	// Variables
	MyVector3 m_startPanPosition;
	MyVector3 m_mousePosition;
	bool threadsStarted = false;
	bool m_exitGame; 
	bool m_mouseClicked;
	bool m_mouseClickedRight;
	bool m_reentry;
	int fpsCounter = 0;
	int m_scroll;
	float currentZoom = 1.0f;
	float agentSpeed = 0.05f;
};

#endif // !GAME

