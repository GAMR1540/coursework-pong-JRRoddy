#include "PowerUpsManager.h"
#include <iostream>


PowerUpsManager::PowerUpsManager(sf::Vector2f fWindowDimensions,std::string sPowerUpFontPath) {

	m_powerUpGenTime = sf::seconds(2.0f); // time it takes to spawn new power ups 
	m_windowWidth = fWindowDimensions.x; /// window width and height variables used for spawning power ups
	m_windowHeight = fWindowDimensions.y;
	m_powerUpTextFont.loadFromFile(sPowerUpFontPath);
	m_textFadeMultiplier = 0.01f; // a low fade druation to ensure the text is always visble for certain time 
	m_powerUpSoundBuffer.loadFromFile(".\\assets\\audio\\PowerUp.wav"); // sound buffer for power ups
	m_powerUpSoundEffect.setBuffer(m_powerUpSoundBuffer); // setting buffer to the m_powerUpSoundBuffer attribute after loading it from the file

}


void PowerUpsManager::handleCollision(Ball*ball,Paddle*player,Paddle*AI) // pass refernce to objects that power up effects are applied to
{
	for (int i = 0; i < m_powerUps.size(); i++) { // for each current power up on the screen

		if (m_powerUps[i].collision(ball->getShapeReference()->getGlobalBounds()) ) { /// if there is a collsion between a power up and the ball
			
			if (m_powerUps[i].hasBall()) { // if the power up has an effect that will be applied to the ball
				searchForExistingBallEffect(m_powerUps[i]); // check if the effect of the power up is already active
			}
			else {
				/// give the power up a refernce to the paddle it may need to apply the effect to based on the velocity of the ball
				ball->getVelocity().x < 0.0f ? m_powerUps[i].setPaddle(AI,-1) : m_powerUps[i].setPaddle(player,1);
				searchForExistingPaddleEffect(m_powerUps[i]); // check if the effect of the power up already exists
			}
			
			addPowerUpText(25, m_powerUps[i]); // add pop up text to the postion that the power up was
			m_powerUpSoundEffect.play(); // play the sound effect assoctaited with power ups
			m_powerUps.erase(m_powerUps.begin() + i); // erase the power up from the vector being used to draw them to the screen 
		}

	}


}

void PowerUpsManager::drawPowerUpText(sf::RenderWindow &window) {
	// drawing pop up text to screen
	for (int i = 0; i < m_activePopUpText.size(); i++) {
		window.draw(m_activePopUpText[i]);
	}
	     
}

void PowerUpsManager::updatePopUpText(float dt) { // update all pop up text on screen

	for (int i = 0; i < m_activePopUpText.size(); i++) { 
		if (m_activePopUpText[i].getFillColor().a <= 0.0f) { // if the alpha is less then 0

			m_activePopUpText.erase(m_activePopUpText.begin() + i);// erase the text object from the vector
			continue;
		}
		sf::Color newPopUpColour = m_activePopUpText[i].getFillColor(); // get the fill colour of the text
		newPopUpColour.a -= (dt*m_textFadeMultiplier); // tick down the alpha value by dt multipler by our fade multiplier
		m_activePopUpText[i].setFillColor(newPopUpColour); // set the new fill colour


	}
}


void PowerUpsManager::searchForExistingBallEffect(PowerUp &newPowerUp) {
	

	for (int i = 0; i < m_activePowerUp.size(); i++) { // for each active power up
	  if (m_activePowerUp[i].getId() == newPowerUp.getId()) { // if there is a ball power up currenlty active with the same effect 
		  
		  m_activePowerUp[i].incrementDuration(); // increment the duration of that current active power up 
		  return; // return as we dont need to push the new power up to the active power ups vector
	  } 
	}
	//otherwise the power up we were checking becomes a new active power up with a ball effect  
	m_activePowerUp.push_back(newPowerUp);

}

// search for exsitig paddle effects 

void PowerUpsManager::searchForExistingPaddleEffect(PowerUp& newPowerUp) {

	for (int i = 0; i < m_activePowerUp.size(); i++) { // for each power up
		// if the effect direction of the power up we are checking is the same as another already in the active power ups vector
		if (m_activePowerUp[i].getEffectDirection() == newPowerUp.getEffectDirection()) { 
			m_activePowerUp[i].incrementDuration(); // we increment the duration of the current active power up of the same type
			return; // we return as we found a power up of the same type 
		}

	}
	// otherwise we push the power up to the active power ups vector setting its effect time before doing so 
	// as we didnt  find an active power up with the same paddle effect
	m_activePowerUp.push_back(newPowerUp);


}

// used to add the pop up text assigned to the power up text vector located in the header file 
void PowerUpsManager::addPowerUpText(unsigned int uiSetCharacterSize,PowerUp &newPowerUp) {

	newPowerUp.getPopUpTextRef()->setPosition(newPowerUp.getShape().getPosition());// used to assign the position of the power up text 
	newPowerUp.getPopUpTextRef()->setCharacterSize(uiSetCharacterSize); // assigning the character size of the power up text
	newPowerUp.getPopUpTextRef()->setFont(m_powerUpTextFont); // assigning the font to the power up pop up text
	m_activePopUpText.push_back(newPowerUp.getPopUpTextValue()); // finally we push the value of the power up text attribute to the power up text vector allowing it to be drawn

}

void PowerUpsManager::manageDurationEffects(Ball*ball,float fDt) { // used to manage any duration effects power ups may have
	
	for (int i = 0; i < m_activePowerUp.size(); i++) { // loop through the active power ups vector

		m_activePowerUp[i].applyEffect(ball,fDt); // apply the power up affect to objects

		if (m_activePowerUp[i].negateEffect(ball)) { // if the power up has fully neagted its effect

			m_activePowerUp.erase(m_activePowerUp.begin() + i);// remove the power up from the active power ups vector
			
		}

	}



}
void PowerUpsManager::resetTimers() { // reset powerUpManager timers
	m_eventTimer.restart();
	m_CollisionTimer.restart();
	m_currentEventTime = m_currentEventTime.Zero;
}

// draw all power ups currenlytin the m_powerups vector to the screen
void PowerUpsManager::draw(sf::RenderWindow&window) {
	for (int i = 0; i < m_powerUps.size(); i++) {
		m_powerUps[i].drawPowerUp(window);
	}
}

void PowerUpsManager::update(float fDt) {

	for (int i = 0; i < m_powerUps.size(); i++) { // loop through the power ups vector

		if (m_powerUps[i].getTargetHitCount() >= m_powerUps[i].getMaxTargetHitCount()) { // if the power up has reached its maximum movement target
			m_powerUps.erase(m_powerUps.begin() + i); /// erase the power up from the current power ups vector
			continue;// move to next itteration
		}
		m_powerUps[i].updatePowerUpPos(fDt); // otherwise update the power up position

	}

}

void PowerUpsManager::generatePowerUp() {

	m_currentEventTime = m_eventTimer.getElapsedTime(); // set the current event timer to the total elapsed time of the sf::clock object used to manage when power ups should spawn
	
	if (m_currentEventTime > m_powerUpGenTime) { // if the current event timer is greater than the end time 

		m_eventTimer.restart(); // restart event clock

		// zero out all other sf::Time objects that are associated with events
		m_currentEventTime = m_currentEventTime.Zero;
		// passing in the window widht and height to the power up constcrtor along with the font that will be used for the text object attribute the power up has 
		PowerUp newPowerUp = PowerUp(m_windowWidth, m_windowHeight);
		// set the type of the power up based on its id in relation to the various durations,colours,and text 
		//associated with attributes of the power ups manager object
		newPowerUp.setType(m_powerUpColours[newPowerUp.getId()], m_effectDurations[newPowerUp.getId()],m_powerUpText[newPowerUp.getId()]);
		newPowerUp.setSpawn(); // set the spawn location of the power up
		m_powerUps.push_back(newPowerUp); // push the power up to the current powerups vector allowing it to be drawn and become active


	}
	



}
void PowerUpsManager::clearPowerUps(Ball*ball) { // clear all power ups on the screen and active power ups

	for (int i = 0; i < m_activePowerUp.size(); i++) { // for each active power up
		m_activePowerUp[i].setTotalEffectTime(sf::Time::Zero);// set its total effect time to zero
		m_activePowerUp[i].negateEffect(ball); // negate the ffect
	}
	m_powerUps.clear();  //clear power up vectors
	m_activePowerUp.clear();
	m_activePopUpText.clear();
	
}


