Paul works by three omniwheels driving a ball underneath. It works with an IMU (MPU-6050) that provides two angles in x and z direction. These two panes are considered separately. The angle is input of a PID controller that delivers a correction speed in the same pane. This speed is converted into a movement of each omniwheel by use of inverse kinematics.  

So, most complex parts of Paul are two things:
1. Kinematics, i.e. having a to-be speed of the bot how do you compute the speed of each omniwheel?
2. Balance Control, i.e. depending on the two angles against the perpendicular axis the IMU provides, how do compute the compensation speed in x and y you send to the kinematics?

Additionally, there is three PWM breakout for the blinking LEDs (everyone likes blinking LEDs!) and an interface two the text2speed module Emic-2 (that's the classical one Steven Hawkins is using). 

The overall components are connected together like this:
<img align="left" width="400px" src="https://github.com/jochenalt/Paul/blob/master/docs/images/architecture.png" >

## Kinematics

I do not list the boring maths here, if you are interested please check the [Kinematics Computation Excel](https://github.com/jochenalt/Paul/blob/master/Mechanics/Ballbot Kinematics.xlsx). The outcome of that is this:

<img align="left" width="400px" src="https://github.com/jochenalt/Paul/blob/master/docs/images/reverse kinematics.png" >

On the left side are the angular speed of the omniwheels 1-3
*r<sub>w</sub>* is the radius of one omniwheel, *r<sub>b</sub>* is the radius of the ball. *θ<sub>3</sub>* is the angle of motors's axis against the horizontal pane. *R<sup>T</sup> is the transposed rotation matrix that is given by the current two angles coming from the IMU:

<img align="left" width="400px" src="https://github.com/jochenalt/Paul/blob/master/docs/images/rotation matrix.png" >

*v<sub>x</sub>* and *v<sub>y</sub>* is the speed in x and y direction, *w<sub>z</sub>* is the angular speed when Paul turns on the same position. All this is implemented in 

<img align="left" width="400px" src="https://github.com/jochenalt/Paul/blob/master/source/BallBot/BallBotController/Kinematics.cpp" >

Micro-wise I used two AVR 644, one for running the balancing loop only, and the other receiving moving commands from the remote via xbee and controlling the LEDs and speed. Since the AVR is a bit too weak to run a controll loop with 100Hz, I used a fixed point arithmentics instead of floats, which makes the code hard to read.

## Construction

The dimensions of the motors, omni wheels and the balls are denoted here:
<img align="left" width="400px" src="https://github.com/jochenalt/Paul/blob/master/docs/images/dimensions.png" >

I did not use a CAD programme but drew all the mechanical parts in powerpoint (really uncool, I know). After printing, I glued that on the plywood and used a scroll saw to get the pieces. This is the [powerpoint file](https://github.com/jochenalt/Paul/blob/master/Mechanics/Construction.ppt)


