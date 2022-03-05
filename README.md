# m5stick-vario
Initial idea was to try to copy the https://github.com/3s1d/tweety and port it on the m5stack. Just to try it out.</br>
The first version working was on the M5Core2 but this platform had way more things than required.</br>
The idee then was to transfert the vario, which i build based on the m5stack-core2, to m5stick-plus-c.</br>

# Code Changes from the Tweety.
The Skytraxx tweety is perfect and i own one and fly Skytraxx. It is perfect for the paragliding.</br>
But i wanted to tinker arround so i took the climb code with the Linear Regression to try to fit it on the M5Stick.</br>

# Flight test.
Well it worked pretty well with the air :-) so i probably stop here.</br>
It is in no way a match with the tweety but it was fun. </br>
One issue which, i can't fix quickly is the polarization of my sunglasses.</br>

# Improvements
* m/s is not calculated correctly.</br>
* duty cycle and duration of the tone is not correct.</br>

# Done
[OK] Simplify the screen (green, red, black), only m/s and maybe an arrow.</br>
[OK] Modify the tone generation to the buzzer.</br>
[NOK] Volume can not be changed. Maximum with PWM is 50% and it is what we did but it is too low.</br>
[OK] Try another external buzzer. You need the hat or external buzzer. Both worked fine.</br>

# Setbacks
* The M5StickC example code is useless. Speakerclass didn't work. Volume was not updating etc...
