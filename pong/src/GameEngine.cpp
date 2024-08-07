#include "GameEngine.h"

GameEngine::GameEngine(sf::RenderWindow& window)
// initilasing all of the attributes that are objects of other classes within the game engine.h file 
	: m_window(window),
	m_paddle1(sf::Vector2f(20, window.getSize().y / 2.f), 10, 100, sf::Color::White),  // initialsing the m_paddle1 class 
	m_paddle2(sf::Vector2f(window.getSize().x - 20.f, window.getSize().y - 100.f), 10, 100, sf::Color::White),
	m_ball(sf::Vector2f(m_window.getSize().x/2.f,m_window.getSize().y/2.f), 8, 400.0f, sf::Color::White),
	m_effects(float(window.getSize().x),float(window.getSize().y)),
	m_powerUpsManager(sf::Vector2f(float(m_window.getSize().x),float(m_window.getSize().y)), ".\\assets\\fonts\\impact.ttf"),
	m_menu(sf::Vector2f(float(m_window.getSize().x),float(m_window.getSize().y)), sf::Vector2f(200.f, 100.f), 25, sf::Color::White, 1.5f)

{   
	srand(time(0)); // set the seed for the sequnce of random numbers for the rand() function to generate(used to randomise things such as coodrinate postions)
	origin = sf::Vector2f(window.getSize().x / 2.f, window.getSize().y / 2.f); /// used for setting objects postion at the centre of the window
	m_p1Score = 0; // keep track of player scores
	m_p2Score = 0;
	m_gStates = GameStates::menu; // on intialisation set the game states enum vairbale to the value of 0
	m_font.loadFromFile(".\\assets\\fonts\\digital-7.ttf");  // used to load the specifc font file that will be used when text is rendered to the screen 
	m_hud.setFont(m_font); // setting the font of the m_hud attribute within the GameEngine class whihc is an object of the Text class 
	m_hud.setCharacterSize(50); 
	m_hud.setFillColor(sf::Color::White);
	
	// loading sound effects and setting buffers for various sounds such as scoring and ball collision
	m_ballBuffer.loadFromFile(".\\assets\\audio\\beep.flac");
	m_ballSound.setBuffer(m_ballBuffer);

	m_gameWinBuffer.loadFromFile(".\\assets\\audio\\winSound.wav");
	m_gameLooseBuffer.loadFromFile(".\\assets\\audio\\looseSound.wav");

	m_scoreSoundBuffer.loadFromFile(".\\assets\\audio\\scoreSound.wav");
	m_scoreSound.setBuffer(m_scoreSoundBuffer);
	
	// loading music used in the menu and during gameplay
	m_introMusic.openFromFile(".\\assets\\audio\\introMusic.wav");
	m_introMusic.setVolume(75.0f);
	m_gameLoopMusic.openFromFile(".\\assets\\audio\\gameLoopMusic.wav"); 
	m_gameLoopMusic.setVolume(75.0f); // lower the volume slighlty so it doesnt drown out other sound effects
	
	m_shouldDrawMenu = true;
	m_hud.setPosition((m_window.getSize().x / 2.f) - 45.f, 10);

	m_paddle1.setSpeed(1000.f);
	
}

void GameEngine::draw()
{
	// draw all shapes and text to the screen
	m_window.clear(); // refresh window for each call to draw ready for the next frame
	m_effects.drawShapes(m_window); // draw all current particle effects to the screen 
	m_effects.drawEventDisplay(m_window); // draw event hud 
	
	m_powerUpsManager.draw(m_window); // draw all power ups to the screen and their pop up text
	m_powerUpsManager.drawPowerUpText(m_window);
	// draw the hud ball, paddles, and menu
	m_paddle1.draw(m_window);
	m_paddle2.draw(m_window);
	m_ball.draw(m_window);
	m_window.draw(m_hud);
	m_menu.draw(m_window,m_shouldDrawMenu);
	m_window.display(); // display everything to the screen once it has been rendered 
}

void GameEngine::resetGame() {
	m_gameEndSound.stop();// ensure the end game sound doesnt continue past the game over screen
	
	// reset game objects to their original positions
	m_ball.resetPos(float(m_window.getSize().x / 2), float(m_window.getSize().y / 2));
	m_paddle1.reset(sf::Vector2f(20, float(m_window.getSize().y / 2)));
	m_paddle2.reset(sf::Vector2f(float(m_window.getSize().x - 20.f),float( m_window.getSize().y - 100.f)));

	m_p1Score = 0;   // also make sure to reset the scores(so it doesnt keep asking them if they want to continue 
	m_p2Score = 0;  // otherwise the gameStates variable would keep getting set to the  gameOver constant 

}

void GameEngine::update()
{
	// update hud
	std::stringstream ss; // string stream that will hold all of the text currently being displayed on the screen 
	switch (m_gStates) // switch case used to determine how the hud should change based on the current value of the enum objet m_gamestates 
	{
	case menu:
		m_window.setMouseCursorVisible(true); // set mouse cursor to be visisble during menu state
		m_ball.getShapeReference()->setFillColor(sf::Color::Black); // ensure that the ball isnt drawn over the menu
		m_menu.Update(m_window);// update the menu fregistering mouse controls and options
		if (m_introMusic.getStatus() == sf::Music::Stopped) { // if the intro music isnt playing or was previously stopped
			m_introMusic.play(); // initate the intro music
			m_introMusic.setLoop(true); // set the intro music to loop while in the intro state and the menu state
		}

		if (m_menu.shouldPlay()) {
			
			m_window.setMouseCursorVisible(false); // set mouse cursor to no longer be visible past the menu state
			m_gStates = intro; // if bool shouldPlay attribiute inside the menu object has been set to true we set the state to intro
			m_shouldDrawMenu = false; // we also set should draw menu to false ensuriong that it isnt drawn on the next frame
			m_menu.resetOptionBools(); // reset any option bools ensuring that they are all false when returning back to the menu after playing
		}
		else if (m_menu.shouldQuit()) { // if the menu option selected was quit
			m_window.close(); // terminate the window 
		}
		break;
	case GameEngine::intro: // if the value of game states has the same value as the enum constant 'intro'(0)
		m_ball.getShapeReference()->setFillColor(sf::Color::White);
		ss << "Press the Space\nkey to start";
		m_effects.resetEventTimer(); // reset the event timers for particle effects
		m_powerUpsManager.resetTimers();// reset the  timers for power ups 

		break;
	case GameEngine::playing: 
		if (m_introMusic.getStatus() != sf::Music::Stopped) { // stop the intro music when we switch to the playing state
			m_introMusic.stop();
		}
		ss << m_p1Score << " - " << m_p2Score;
		break;

	case GameEngine::gameOver:
		if (m_p1Score > m_p2Score) { // if player 1 score is greater than the ai score

			ss << "You win !\n"; // using the insertion operator refershe the string stream used for controlling the huf with the new input message

		
		}
		else { // otherwise similar process to above but with computer win message
			ss << "Computer wins...\n";
			

		}
		
		// allow player to continue after winning or losing 
		
		ss << "would you like to\n continue(y/n)\n";  // added continue message during the game over screen
		
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Y)) {  // if the m_gameStates is currently set to the enum constant "gameOver" and the user presses the Y key
			m_gStates = intro;  // we take them back to the intro state/screen allowing them to play again(rather than just closing the program
			resetGame(); // reset object positions
			
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::N)) { // if the user presses N during the game over screen
			m_gStates = menu; // take the player back to the menu
			m_shouldDrawMenu = true; // set the bool for drawning the menu to true
			resetGame(); // reset all object positions

		}
		

		break;

	default:
		break;
	}
		
	m_hud.setString(ss.str()); // assigining the string stream to the m_hud attribute that is an object of the built in SFML Text class that allows text to be drawn to the window
}


void GameEngine::run()
{
	float dt;
	
	while (m_window.isOpen())
	{
		dt = m_clock.restart().asSeconds(); // get the differnce between the last and current frame(delta time)  
		//allowing better control of how the game updates interms of moving objects etc(ensuring that it is more consistent across different frame rates)

		sf::Event event;// creating an event object tied to the smfl library that allows handling of things such as key presses 
		while (m_window.pollEvent(event)) 
		{
			// using the window object's pollEvent method as the condition for the while loop continuesly 
			//checking for certain events such as keyboard input or the window closing  {
			
			if (event.type == sf::Event::Closed) m_window.close(); 
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) // close the window using escape
				m_window.close();

			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space && m_gStates == intro )
				m_gStates = GameStates::playing;
		}
		


		// ADD YOUR CODE HERE !!!

		

		if (m_gStates == 1 ) { // check if the game should be running using the m_gStates enum variable
			

			if (m_gameLoopMusic.getStatus() == sf::Music::Stopped) {// if the status is currently stopped i.e we are first enetring the loop
				m_gameLoopMusic.play(); // play the game loop music
				m_gameLoopMusic.setLoop(true); // set music to loop 
			}
			// allow the user to move the left paddle with both arrow keys and W/S
			if ( sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up)){ // using the SFML isKeyPressed method from the keyBoard class to detect when a specifc key is pressed based on the enum for the key passed in
				m_paddle1.move(-dt , float(m_window.getSize().y)); // if the condtion above is true move the player up
			
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
				m_paddle1.move(dt, float(m_window.getSize().y));// if the condtion above is true move the player down
				
			}
			
			// get the ball to  move 
			m_ball.move(dt, m_window); 
			m_ball.increaseSpeed(dt);
			// below is a tracking method created for the paddle class which
			//allows the AI paddle to track the ball based on the vector between the ball and the paddle 
			/// get the second paddle to track the ball 
			m_paddle2.trackBall(m_ball.getPosition(), m_ball.getVelocity(), dt, float(m_window.getSize().y));
			
			
			//// collsion detection  for both paddles 
			if (m_ball.ballCollisionPushBack(m_paddle1.getShape())) { // if the global bounds of the paddle contain the balls position meaning that the two bounding rectangles would over lap
				
				if (m_ballSound.getStatus() != sf::Sound::Playing) {// if the collison sound is not currenlty playing
					m_ballSound.play();// initiate the sound 
				}
				m_effects.generateCollsionParticles(m_ball.getPosition(), 1, m_paddle1.getShape().getFillColor(),5); // generate particles when the ball collides with the paddle defining the postion they start at and the direction of movement(negative or positive)
			   
			}
			if (m_ball.ballCollisionPushBack(m_paddle2.getShape())){ // if the global bounds of the paddle contain the balls position meaning that the two bounding rectangles would over lap 
				
				if (m_ballSound.getStatus() != sf::Sound::Playing) {  // if the collison sound isnt currently playing
					m_ballSound.play(); 
				}
				m_effects.generateCollsionParticles(m_ball.getPosition(), -1, m_paddle2.getShape().getFillColor(),5); // generate particles when the ball collides with the paddle defining the postion they start at and the direction of movement(negative or positive)
			}
			
			// updating scores when the ball passes either paddle 

			if (m_ball.getPosition().x > m_window.getSize().x + m_ball.getShape().getRadius()) { // check if the ball has gone passed the paddles position 
				
				
				m_scoreSound.play(); // play the score sound effect
				float fnewPosY = m_screenRandomBoundLower + static_cast<float>(rand())  / static_cast<float>((RAND_MAX / (m_screenRandomBoundUpper - m_screenRandomBoundLower)));
				m_ball.resetPos(origin.x, fnewPosY); // reset balls postion
				m_p1Score++;  // increment the score attribute of the m_paddle1 object(as it just scored)
				m_paddle2.aiValidateScore(float(m_p1Score),float( m_p2Score),float( m_maxScore)); // revalidate ai speed

			}
			if (m_ball.getPosition().x < 0 - m_ball.getShape().getRadius()) { // similar process as described above but for when the ball passes the left/player paddle 
				
				m_scoreSound.play();

				float fnewPosY = m_screenRandomBoundLower + static_cast<float>(rand())  / static_cast<float>((RAND_MAX / (m_screenRandomBoundUpper - m_screenRandomBoundLower)));
				m_ball.resetPos( origin.x, fnewPosY); // reset balls postion
				m_p2Score++;
				
				m_paddle2.aiValidateScore(float(m_p1Score), float(m_p2Score), float(m_maxScore)); // have the ai re validate the score adjusting its speed to the current status of the game
			}

			

			m_powerUpsManager.generatePowerUp(); // control when power ups spawn
			m_powerUpsManager.update(dt); // update power up positions 
			m_powerUpsManager.handleCollision(&m_ball,&m_paddle1,&m_paddle2); // handle collisions and effect application 
			m_powerUpsManager.manageDurationEffects(&m_ball,dt); // handle duration effects
			m_powerUpsManager.updatePopUpText(dt);


			m_effects.generateEvent(dt); // when particle effect event should trigger
			m_effects.manageEvents();// manage when particle event triggers
			m_effects.update(dt); // update all particles currently on screen 
			m_effects.handleParticleCollisions(&m_ball);
			
			
			// checking for if either paddle has a score equal to the m_maxScore variable
			if (m_p1Score == m_maxScore || m_p2Score == m_maxScore) { // check if either score attribute attached to the paddle 1 and paddle 2 objects has reached the max score count 

				m_gStates = gameOver; // if so set the current value of m_gamestates to the constant "gameOver" defined in the enum type gameStates(in the GameEngine header file)
				m_gameLoopMusic.stop();
				
				m_effects.clearParticle(); // clear all particles if there are any remaining on screen 
				
				m_effects.resetEventTimer();
				// reset powerups
				m_powerUpsManager.clearPowerUps(&m_ball);
				m_powerUpsManager.resetTimers();
				// detminign which sound buffer should be allocated to the m_gameEndSound attribute 
				m_p1Score > m_p2Score ? m_gameEndSound.setBuffer(m_gameWinBuffer) : m_gameEndSound.setBuffer(m_gameLooseBuffer);
				m_gameEndSound.play();
			}

			
		}
		// update hud
		update();
		// draw shapes to screen
		draw();
	}
}
