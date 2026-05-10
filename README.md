# RotatingDisplayTableArduino
I am making a rotating table with Arduino for displaying a design object in an exhibition.
My purpose is to use UltraSonic sensor to detect the visitors and trigger the table to rotate when someone is within the distance.
For the motor, I use a NEMA 17 stepper motor 42BYGH33. My Arduino code is as following. I am using the AccelStepper library to control a stepper motor through the TMC2208 controller. 
