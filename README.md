## TemperatureAlarm
This project simulates a temperature detecting alarm system on  a Keysight U3810A IoT kit.

The user sets a preset temperature and while monitoring, when the temperature is above the preset value the user is notified by a blinking LED to disarm the alarm. After 1 minute if the user does not notice the LED and disarm the alarm the LED blinks faster. After 2 minutes the fan/relay is turned on while
the LED still blinks.

The logic of the program :
 - State 0: The user is asked to input the preset temperature
  i. B1 to increase, B2 to decrease and B4 to save
 - State 1: The preset temperature is saved, the temperature sensor starts monitoring the temperature
 - State 2: The temperature is higher than the preset temperature, the LED startsblinking at 2Hz
 - State 3: 1 minute has elapsed without the alarm disarmed, the LED blinks faster at 10Hz
 - State 4: 2 minutes have elapsed without the alarm disarmed, the relay is activated while the LED still blinks at 10Hz
  
  
 <img width="659" alt="Screen Shot 2022-08-22 at 8 36 17 PM" src="https://user-images.githubusercontent.com/61993180/186043239-db7bde80-1775-4fbe-b3fd-041841ce4198.png">
