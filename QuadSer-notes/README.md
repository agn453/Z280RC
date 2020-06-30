Notes regarding QuadSer modification
------------------------------------

Bill Shen's QuadSer 4-port UART module is subject to spurious
interrupts due to floating inputs on unused input pins.

Hector Peraza discovered this while porting his terminal driver
for RSX280 to support this board.

The "work-around"/fix is to add some small resistors across the
receiver inputs on each channel to ground.  This lowers the input
impedance and shunts induced noise to ground, and does not interfere
with normal operation.

Both Hector and I used 10Kohm 0.1W radial lead resistors - you'll need
four of these, and they're soldered directly to the rear of the QuadSer
Rev1 module that has already been modified for the Z280RC.

Hector provided a photo here with the resistors in place (highlighted).

[https://github.com/agn453/Z280RC/blob/master/QuadSer-notes/QuadSer-10Kohm-pullups.jpg|alt=Pullups]

