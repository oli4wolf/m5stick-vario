# m5stick-vario
The idee is to transfert the vario, which i build based on the m5stack-core2, to m5stick-plus-c.

# Differences
* Removed the sdcard.
* Different algorithm for the beep.

# TODO
[OK] Modify Platformio for m5stick.
[OK] Adapt the Pin adresses for the groove connector.
[OK] Simplify the screen (green, red, black), only m/s and maybe an arrow.
[OK] Modify the tone generation to the buzzer.
[] Volume can not be changed.
[] Tone does not stop after duration.
[] m/s is not calculated correctly.
[] duty cycle and duration of the tone is not correct.

Piezo verwendet PKLCS1212 -> 70 - 85db bei 10cm