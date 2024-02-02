# A Useless Button
Example of a useless button. It doesn't do anything good except being great at what it does.
It demonstrates the power of repetition: if you repeat the samething many times over, it becomes you.

## how to use
If the user press the button once, they affirm what is displayed. 
Pressing it twice will double the affirmation, and the alignment with the affirmed content intensifies with each subsequent press.

## How it works 
The program employs a state machine to monitor button states, facilitating the detection of single or double button presses—a kind of software debouncer. When a button is pressed and released, it doesn't simply transition states twice; instead, it fluctuates multiple times due to the mechanical vibrations. It's hard to see unless you have a 3rd eye. The software debouncer incorporates a brief duration during which button readings are disregarded, allowing the button sufficient time to stabilize and the vibrations to settle before resuming the assessment of button states.

## circuit
Wire a button to PC0 and GND
wire a LED to PD0 and GND