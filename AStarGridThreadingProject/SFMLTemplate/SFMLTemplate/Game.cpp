#include "Game.h"

Game::Game() :
	m_window{ sf::VideoMode{ 2000, 1600, 32 }, "Games Engineering Project" },
	m_exitGame{ false }

{
	// Fps text
	m_font.loadFromFile("BBrick.ttf");
	m_fps.setPosition(100, 100);
	m_fps.setFillColor(sf::Color::Blue);
	m_fps.setCharacterSize(30);
	m_fps.setString("50");
	m_fps.setFont(m_font);

	srand(time(nullptr));
	// Setup the Map to begin with
	setupMedium(50);
}

Game::~Game()
{
}

void Game::run()
{
	sf::Clock clock;
	sf::Time timeSinceLastUpdate = sf::Time::Zero;
	sf::Time timePerFrame = sf::seconds(1.f / 3000.f); // 60 fps
	while (m_window.isOpen())
	{
		processEvents(); 
		update(timePerFrame);
		fpsDelay += fpsClock.restart();
		if (fpsDelay >= sf::seconds(1))		// Update FPS text every second
		{
			m_fps.setString(std::to_string(fpsCounter));
			fpsDelay = sf::seconds(0);
			fpsCounter = 0;
		}
		render(); 
		fpsCounter++;
	}
}

void Game::processEvents()
{
	while (m_window.pollEvent(event))
	{
		if (sf::Event::Closed == event.type) // window message
		{
			m_window.close();
		}
		if (sf::Event::KeyPressed == event.type) //user key press
		{
			if (sf::Keyboard::Escape == event.key.code)
			{
				m_exitGame = true;
			}
		}
		if (event.type == sf::Event::MouseButtonPressed)
		{
			if (event.mouseButton.button == sf::Mouse::Left)
			{
				m_mouseClicked = true;
			}
			if (event.mouseButton.button == sf::Mouse::Right)
			{
				m_mouseClickedRight = true;
			}
		}
		if (event.type == sf::Event::MouseButtonReleased)
		{
			if (event.mouseButton.button == sf::Mouse::Left)
			{
				m_mouseClicked = false;
			}
			if (event.mouseButton.button == sf::Mouse::Right)
			{
				m_mouseClickedRight = false;
			}
		}
		if (event.type == sf::Event::MouseWheelMoved)
		{
			if (m_scroll != event.mouseWheel.delta)
				m_scroll = event.mouseWheel.delta;
			else
				m_scroll = 0;
		}
	}
}

void Game::update(sf::Time t_deltaTime)
{
	if (m_exitGame)
	{
		m_window.close();		// Exiting the game
	}
	// Update time managment
	m_moveTimer += timeManager.restart();
	m_clickTimer += timeManager2.restart();

	// Get mouse position for view alterations
	m_mousePosition = sf::Mouse::getPosition(m_window);

	// Start the pathfinding
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && threadsStarted == false && m_clickTimer.asSeconds() > 0.25f)
	{
		m_clickTimer = sf::seconds(0);		// reset timer
		m_finishedIds.clear();				// Clear info
		threadsStarted = true;
		for (int i = 0; i < numOfAgents; i++)		// For every Agent
		{
			std::vector<Node*>* P = new std::vector<Node*>();			
			paths.push_back(P);
			skipCounter.push_back(0);												// For managing later
			Node* start = m_gameNodes.at(m_gameNodes.size()/2);						// Left side middle
			auto functionPtr = std::bind(&Game::aStar, this, start, m_goal, paths.at(i), i);	// Bind the Astar function with it's params and index
			threadpool.addTask(functionPtr);
		}
	}

	// Small Map
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::R) && m_clickTimer.asSeconds() > 0.25f)
	{
		m_clickTimer = sf::seconds(0);
		setupSmall();
	}
	// Medium Map
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::T) && m_clickTimer.asSeconds() > 0.25f)
	{
		m_clickTimer = sf::seconds(0);
		setupMedium(50);
		doRetro = false;
	}
	// Medium Map 500
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::G) && m_clickTimer.asSeconds() > 0.25f)
	{
		m_clickTimer = sf::seconds(0);
		setupMedium(500);
		doRetro = true;
	}
	// Large Map 
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Y) && m_clickTimer.asSeconds() > 0.25f)
	{
		setupLarge();
	}

	// Increase Agent Speed
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) )
	{
		if ((agentSpeed - 0.0001f) > 0.01f)
		{
			agentSpeed -= 0.0001f;
		}
	}
	// Decrease Agent Speed
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
	{
		if ((agentSpeed + 0.0001f) < 1.0f)
		{
			agentSpeed += 0.0001f;
		}
	}

	// Update the agents when they have pathfinded
	if (m_moveTimer.asSeconds() > agentSpeed) // timer
	{
		m_moveTimer = sf::seconds(0);
		for (int l = 0; l < numOfAgents; l++)	// For every agent
		{
			for (auto id : m_finishedIds)		// For every finsihed ID
			{
				if (id == l)					// If that agents id is finishbed
				{
					if (paths.at(l)->at(m_nexts.at(l))->occupied == false || (m_nexts.at(l) == paths.at(l)->size() - 1) || skipCounter.at(l) > 25)	// IF next move is valid
					{
						skipCounter.at(l) = 0;
						paths.at(l)->at(m_nexts.at(l))->occupied = true;			// Set the node to occupied
						if (m_nexts.at(l) > 0)
							paths.at(l)->at(m_nexts.at(l) - 1)->occupied = false;	// Unset

						m_agents.at(l)->setPosition(paths.at(l)->at(m_nexts.at(l))->position.x * 10, paths.at(l)->at(m_nexts.at(l))->position.y * 10);		// Move to the next
						if (m_nexts.at(l) < paths.at(l)->size() - 1)
							m_nexts.at(l)++;							// Set next move
					}
					else
					{
						skipCounter.at(l)++;
					}
				}
			}
		}
	}

	// Controlling the camera
 	if (m_scroll != 0)
	{
		if (m_scroll < 0)
		{
			m_camera.zoom(1.1f);
			currentZoom *= 1.1;
			if (m_fps.getCharacterSize() < 1000)
				m_fps.setCharacterSize(m_fps.getCharacterSize() + 2);
		}
		else
		{
			m_camera.zoom(0.9f);
			currentZoom *= 0.9;
			if(m_fps.getCharacterSize() > 2)
				m_fps.setCharacterSize(m_fps.getCharacterSize() - 2);
		}
	}
	m_scroll = 0;
	if (m_mouseClickedRight == true )
	{
		rightClickScroll(m_mousePosition);
	}
	else
	{
		m_reentry = false;
		m_startPanPosition = m_mousePosition;
	}
	m_fps.setPosition((m_camera.getCenter() - sf::Vector2f(500 * currentZoom, 500* currentZoom)));
}

void Game::render()
{
	m_window.clear(sf::Color::Black);
	m_window.setView(m_camera);

	sf::Sprite sprite(m_gameMap.getTexture());
	m_window.draw(sprite);

	m_window.draw(m_goalSquare);

	for (int t = 0; t < numOfAgents; t++)
	{
		m_window.draw(*m_agents.at(t));
	}
	m_window.draw(m_fps);
	m_window.display();
}

void Game::rightClickScroll(MyVector3 mouseClick)
{
	if (m_reentry == false)
	{
		m_startPanPosition = mouseClick;
		m_reentry = true;
	}

	MyVector3 distanceFromPan = mouseClick - m_startPanPosition;
	float power = distanceFromPan.length();
	if (power > 8)
	{
		power = 8;
	}
	distanceFromPan.normalise();
	sf::Vector2f dist = -distanceFromPan;
	Game::m_camera.setCenter(Game::m_camera.getCenter() + dist * (power / 10));
}

void Game::reset()
{
	skipCounter.clear();
	for (auto& a : m_agents)
	{
		delete a;
	}
	m_agents.clear();
	for (auto& a : m_tiles)
	{
		delete a;
	}
	m_tiles.clear();
	for (auto& a : m_gameNodes)
	{
		delete a;
	}
	m_gameNodes.clear();
	m_nexts.clear();
	for (auto& a : paths)
	{
		delete a;
	}
	paths.clear();
	m_finishedIds.clear();
	threadsStarted = false;
}


void Game::aStar(Node* t_start, Node* t_end, std::vector<Node*>* path, int index)
{
	Node* starty = t_start;
	Node* goal = t_end;
	float ambush;
	int baseArcWeight = 10;

	// Lambda for comparing the total value
	auto costComparer = [&](Node* node1, Node* node2)
	{
		return node1->totalCostAccumlative.at(index) > node2->totalCostAccumlative.at(index);
	};
	std::priority_queue<Node*, std::vector<Node*>, decltype(costComparer)> pq(costComparer);

	starty->totalCostFromStart.at(index) = 0;
	for (auto& n : m_gameNodes)
	{
		n->costToGoal.at(index) = (goal->position - n->position).length();
		n->totalCostAccumlative.at(index) = std::numeric_limits<int>::max();
	}
	pq.push(starty);
	starty->marked.at(index) = true;
	starty->totalCostAccumlative.at(index) = starty->totalCostFromStart.at(index) + starty->costToGoal.at(index);
	while (pq.size() > 0 && pq.top() != goal)
	{
		auto iter = pq.top()->arcs.begin();			// Iterators of each arc of the current front of the queue, each will be marked then added to the Queue if they aren't the goal
		auto endIter = pq.top()->arcs.end();
		
		for (; iter != endIter; iter++)					// Until all children checked
		{
			if ((*iter)->passable == false)
			{
				continue;
			}

			ambush = pq.top()->totalCostFromStart.at(index) + baseArcWeight + (((*iter)->numberOfProcessed) * ((*iter)->numberOfProcessed));		// New ambush value
			float childDist = (*iter)->costToGoal.at(index) + ambush;																				// Child cost
			if (childDist < (*iter)->totalCostAccumlative.at(index))
			{
				(*iter)->totalCostAccumlative.at(index) = childDist;
				(*iter)->totalCostFromStart.at(index) = ambush;
				(*iter)->previous.at(index) = pq.top();
			
				if (doRetro == false)
				{
					std::unique_lock<std::mutex> lock(ambushLock);		// Lock to avoid issues
					(*iter)->numberOfProcessed++;						// Increase this value so other agents will work better
				}
			}
	
			// Marking
			if ((*iter)->marked.at(index) == false)				// Push to Queue if it hasn't been already
			{
				pq.push((*iter));
				(*iter)->marked.at(index) = true;
			}
		}
		pq.pop();
	}

	// Create Path ------------------------------------
	path->push_back(goal);
	Node* loop = goal;
	while (loop->previous.at(index) != nullptr && loop->previous.at(index) != t_start)
	{
		loop = loop->previous.at(index);
		if (doRetro == true)
		{
			std::unique_lock<std::mutex> lock(ambushLock);	// Lock to avoid issues
			loop->numberOfProcessed++;						// If looking back retrospectively, gives much better results
		}
		path->push_back(loop);
	}
	std::reverse(path->begin(), path->end());
	finished(index);
	// -------------------------------------------------
}

void Game::finished(int t_index)
{
	{
		std::unique_lock<std::mutex> lock(finishedLock);	// Lock to avoid issues
		m_finishedIds.push_back(t_index);
	}
	std::cout << "Finsihed Agent: " <<  std::to_string(t_index) << std::endl;
}

void Game::setupSmall()
{
	gridSize = 30;
	gridSizeTotal = gridSize * gridSize;
	numOfAgents = 5;
	
	reset();

	m_gameMap.create(gridSize * 10, gridSize * 10);
	m_gameMap.clear(sf::Color::Black);
	int h2 = 0;
	bool grey = false;
	for (int i = 0; i < gridSize; i++)
	{
		grey = !grey;
		for (int j = 0; j < gridSize; j++)
		{
			grey = !grey;
			sf::RectangleShape* newTile = new sf::RectangleShape();
			if (grey)
				newTile->setFillColor(sf::Color(150, 150, 150, 255));
			else
				newTile->setFillColor(sf::Color(255, 255, 255, 255));

			newTile->setPosition(j * 10, i * 10);
			newTile->setSize(sf::Vector2f(10, 10));

			Node* node = new Node;

			node->passable = 1;

			if (j == 6 && i < 20)
			{
				node->passable = 0;
				newTile->setFillColor(sf::Color(0, 0, 0, 255));
			}

			if (j == 12 && (i > 6 && i < 25))
			{
				node->passable = 0;
				newTile->setFillColor(sf::Color(0, 0, 0, 255));
			}

			if (j == 20 && (i > 3 && i < 27))
			{
				node->passable = 0;
				newTile->setFillColor(sf::Color(0, 0, 0, 255));
			}
			
			m_gameMap.draw(*newTile);
			m_tiles.push_back(newTile);

			node->ID = (h2);
			node->numberOfProcessed = 0;
			node->position.x = j;
			node->position.y = i;
			node->costToGoal.resize(numOfAgents);
			node->totalCostAccumlative.resize(numOfAgents);
			node->totalCostFromStart.resize(numOfAgents);
			node->previous.resize(numOfAgents);
			node->marked.resize(numOfAgents);
			for (int t = 0; t < numOfAgents; t++)
			{
				node->costToGoal.at(t) = 1;
				node->totalCostAccumlative.at(t) = 10000;
				node->totalCostFromStart.at(t) = 0;
				node->previous.at(t) = nullptr;
				node->marked.at(t) = 0;
			}

			if (j != 0)
				node->arcIDs[0] = ((h2)-1);
			else
				node->arcIDs[0] = -1;

			// Right
			if (j != (gridSize - 1))
				node->arcIDs[1] = ((h2)+1);
			else
				node->arcIDs[1] = -1;

			// Up
			if (i != 0)
				node->arcIDs[2] = ((h2)-(gridSize));
			else
				node->arcIDs[2] = -1;

			// Down
			if (i != (gridSize - 1))
				node->arcIDs[3] = ((h2)+(gridSize));
			else
				node->arcIDs[3] = -1;

			m_gameNodes.push_back(node);
			h2++;
		}
	}

	for (auto& nod : m_gameNodes)
	{
		for (int i = 0; i < 4; i++)
		{
			if (nod->arcIDs[i] != -1)
			{
				nod->arcs.push_back(m_gameNodes.at(nod->arcIDs[i]));
			}
		}
	}
	m_nexts.resize(numOfAgents);
	for (int t = 0; t < numOfAgents; t++)
	{
		sf::RectangleShape* newTile = new sf::RectangleShape();
		newTile->setFillColor(sf::Color::Red);
		newTile->setPosition(0 * 10, 15 * 10);
		newTile->setSize(sf::Vector2f(10, 10));
		newTile->setOutlineThickness(1);
		newTile->setOutlineColor(sf::Color::Black);
		m_agents.push_back(newTile);
		m_nexts.at(t) == 0;
	}

	m_goal = m_gameNodes.at(329);
	m_goalSquare.setFillColor(sf::Color::Blue);
	m_goalSquare.setPosition(m_goal->position.x * 10, m_goal->position.y * 10);
	m_goalSquare.setSize(sf::Vector2f(10, 10));

	m_gameMap.display();
}

void Game::setupMedium(int t_numberOfAgents)
{
	gridSize = 100;
	gridSizeTotal = gridSize * gridSize;
	numOfAgents = t_numberOfAgents;

	reset();

	m_gameMap.create(gridSize * 10, gridSize * 10);
	m_gameMap.clear(sf::Color::Black);
	int h2 = 0;
	bool grey = false;
	for (int i = 0; i < gridSize; i++)
	{
		grey = !grey;
		for (int j = 0; j < gridSize; j++)
		{
			grey = !grey;
			sf::RectangleShape* newTile = new sf::RectangleShape();
			if (grey)
				newTile->setFillColor(sf::Color(150, 150, 150, 255));
			else
				newTile->setFillColor(sf::Color(255, 255, 255, 255));

			newTile->setPosition(j * 10, i * 10);
			newTile->setSize(sf::Vector2f(10, 10));

			Node* node = new Node;

			node->passable = 1;

			if (j == 20 && (i > 15 && i < 85))
			{
				node->passable = 0;
				newTile->setFillColor(sf::Color(0, 0, 0, 255));
			}

			if (j == 30 && (i > 35 && i < 85))
			{
				node->passable = 0;
				newTile->setFillColor(sf::Color(0, 0, 0, 255));
			}

			if (j == 30 && (i > 3 && i < 27))
			{
				node->passable = 0;
				newTile->setFillColor(sf::Color(0, 0, 0, 255));
			}

			if (j == 40 && (i > 10 && i < 80))
			{
				node->passable = 0;
				newTile->setFillColor(sf::Color(0, 0, 0, 255));
			}

			if (j == 50 && (i < 65 || i > 80))
			{
				node->passable = 0;
				newTile->setFillColor(sf::Color(0, 0, 0, 255));
			}


			if (j == 60 && (i > 15 && i < 85))
			{
				node->passable = 0;
				newTile->setFillColor(sf::Color(0, 0, 0, 255));
			}

			if (j == 70 && i > 35)
			{
				node->passable = 0;
				newTile->setFillColor(sf::Color(0, 0, 0, 255));
			}

			if (j == 85 && (i > 10 && i < 75))
			{
				node->passable = 0;
				newTile->setFillColor(sf::Color(0, 0, 0, 255));
			}

			m_gameMap.draw(*newTile);
			m_tiles.push_back(newTile);

			node->numberOfProcessed = 0;
			node->ID = (h2);
			node->position.x = j;
			node->position.y = i;
			node->costToGoal.resize(numOfAgents);
			node->totalCostAccumlative.resize(numOfAgents);
			node->totalCostFromStart.resize(numOfAgents);
			node->previous.resize(numOfAgents);
			node->marked.resize(numOfAgents);
			for (int t = 0; t < numOfAgents; t++)
			{
				node->costToGoal.at(t) = 1;
				node->totalCostAccumlative.at(t) = 10000;
				node->totalCostFromStart.at(t) = 0;
				node->previous.at(t) = nullptr;
				node->marked.at(t) = 0;
			}

			if (j != 0)
				node->arcIDs[0] = ((h2)-1);
			else
				node->arcIDs[0] = -1;

			// Right
			if (j != (gridSize - 1))
				node->arcIDs[1] = ((h2)+1);
			else
				node->arcIDs[1] = -1;

			// Up
			if (i != 0)
				node->arcIDs[2] = ((h2)-(gridSize));
			else
				node->arcIDs[2] = -1;

			// Down
			if (i != (gridSize - 1))
				node->arcIDs[3] = ((h2)+(gridSize));
			else
				node->arcIDs[3] = -1;

			m_gameNodes.push_back(node);
			h2++;
		}
	}

	for (auto& nod : m_gameNodes)
	{
		for (int i = 0; i < 4; i++)
		{
			if (nod->arcIDs[i] != -1)
			{
				nod->arcs.push_back(m_gameNodes.at(nod->arcIDs[i]));
			}
		}
	}
	m_nexts.resize(numOfAgents);
	for (int t = 0; t < numOfAgents; t++)
	{
		sf::RectangleShape* newTile = new sf::RectangleShape();
		newTile->setFillColor(sf::Color::Red);
		newTile->setPosition(0 * 10, 50 * 10);
		newTile->setSize(sf::Vector2f(10, 10));
		newTile->setOutlineThickness(1);
		newTile->setOutlineColor(sf::Color::Black);
		m_agents.push_back(newTile);
		m_nexts.at(t) == 0;
	}

	m_goal = m_gameNodes.at(4999);
	m_goalSquare.setFillColor(sf::Color::Blue);
	m_goalSquare.setPosition(m_goal->position.x * 10, m_goal->position.y * 10);
	m_goalSquare.setSize(sf::Vector2f(10, 10));

	m_gameMap.display();
}

void Game::setupLarge()
{
	gridSize = 1000;
	gridSizeTotal = gridSize * gridSize;
	numOfAgents = 50;

	reset();

	m_gameMap.create(gridSize * 10, gridSize * 10);
	m_gameMap.clear(sf::Color::Black);
	std::vector<int> randomsUpper;
	std::vector<int> randomsLower;
	for (int g = 50; g < 1000; g += 50)
	{
		int rn = rand() % 100;
		randomsUpper.push_back(rn);
		int rn2 = rand() % 100;
		randomsLower.push_back(rn2);
	}

	int h2 = 0;
	bool grey = false;
	for (int i = 0; i < gridSize; i++)
	{
		grey = !grey;
		for (int j = 0; j < gridSize; j++)
		{
			grey = !grey;
			sf::RectangleShape* newTile = new sf::RectangleShape();
			if (grey)
				newTile->setFillColor(sf::Color(150, 150, 150, 255));
			else
				newTile->setFillColor(sf::Color(255, 255, 255, 255));

			newTile->setPosition(j * 10, i * 10);
			newTile->setSize(sf::Vector2f(10, 10));

			Node* node = new Node;

			node->passable = 1;
			int dex = 0;
			for (int b = 50; b < 1000; b += 50)
			{
				if (j == b && (i > (150 - randomsLower.at(dex)) && i < (850 + randomsUpper.at(dex))))
				{
					node->passable = 0;
					newTile->setFillColor(sf::Color(0, 0, 0, 255));
				}
				dex++;
			}
	
			if (j == 200 && i < 500)
			{
				node->passable = 0;
				newTile->setFillColor(sf::Color(0, 0, 0, 255));
			}

			if (j == 400 && i > 500)
			{
				node->passable = 0;
				newTile->setFillColor(sf::Color(0, 0, 0, 255));
			}

			if (j == 600 && i < 500)
			{
				node->passable = 0;
				newTile->setFillColor(sf::Color(0, 0, 0, 255));
			}

			if (j == 800 && i > 500)
			{
				node->passable = 0;
				newTile->setFillColor(sf::Color(0, 0, 0, 255));
			}

			m_gameMap.draw(*newTile);
			m_tiles.push_back(newTile);

			node->ID = (h2);
			node->numberOfProcessed = 0;
			node->position.x = j;
			node->position.y = i;
			node->costToGoal.resize(numOfAgents);
			node->totalCostAccumlative.resize(numOfAgents);
			node->totalCostFromStart.resize(numOfAgents);
			node->previous.resize(numOfAgents);
			node->marked.resize(numOfAgents);
			for (int t = 0; t < numOfAgents; t++)
			{
				node->costToGoal.at(t) = 1;
				node->totalCostAccumlative.at(t) = 10000;
				node->totalCostFromStart.at(t) = 0;
				node->previous.at(t) = nullptr;
				node->marked.at(t) = 0;
			}

			if (j != 0)
				node->arcIDs[0] = ((h2)-1);
			else
				node->arcIDs[0] = -1;

			// Right
			if (j != (gridSize - 1))
				node->arcIDs[1] = ((h2)+1);
			else
				node->arcIDs[1] = -1;

			// Up
			if (i != 0)
				node->arcIDs[2] = ((h2)-(gridSize));
			else
				node->arcIDs[2] = -1;

			// Down
			if (i != (gridSize - 1))
				node->arcIDs[3] = ((h2)+(gridSize));
			else
				node->arcIDs[3] = -1;

			m_gameNodes.push_back(node);
			h2++;
		}
	}

	for (auto& nod : m_gameNodes)
	{
		for (int i = 0; i < 4; i++)
		{
			if (nod->arcIDs[i] != -1)
			{
				nod->arcs.push_back(m_gameNodes.at(nod->arcIDs[i]));
			}
		}
	}
	m_nexts.resize(numOfAgents);
	for (int t = 0; t < numOfAgents; t++)
	{
		sf::RectangleShape* newTile = new sf::RectangleShape();
		newTile->setFillColor(sf::Color::Red);
		newTile->setPosition(0 * 10, 500 * 10);
		newTile->setSize(sf::Vector2f(10, 10));
		newTile->setOutlineThickness(1);
		newTile->setOutlineColor(sf::Color::Black);
		m_agents.push_back(newTile);
		m_nexts.at(t) == 0;
	}

	m_goal = m_gameNodes.at(499999);
	m_goalSquare.setFillColor(sf::Color::Blue);
	m_goalSquare.setPosition(m_goal->position.x * 10, m_goal->position.y * 10);
	m_goalSquare.setSize(sf::Vector2f(10, 10));

	m_gameMap.display();
}
