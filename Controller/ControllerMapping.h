#ifndef CONTROLLERMAPPING_H_
#define CONTROLLERMAPPING_H_

//TODO: onPress, on release, on hold
/* TODO: joyticks
if((buttons.ljoy_h <= 50) || (buttons.ljoy_h >= 200)) printf("Left Analog  X: %i\n", (int)buttons.ljoy_h);
if((buttons.ljoy_v <= 50) || (buttons.ljoy_v >= 200)) printf("Left Analog  Y: %i\n", (int)buttons.ljoy_v);
if((buttons.rjoy_h <= 50) || (buttons.rjoy_h >= 200)) printf("Right Analog X: %i\n", (int)buttons.rjoy_h);
if((buttons.rjoy_v <= 50) || (buttons.rjoy_v >= 200)) printf("Right Analog Y: %i\n", (int)buttons.rjoy_v);
 */

typedef void *onStart();
typedef void *onSelect();

typedef void *onCross();
typedef void *onCircle();
typedef void *onTriangle();
typedef void *onSquare();

typedef void *onLeft();
typedef void *onRight();
typedef void *onUp();
typedef void *onDown();

typedef void *onL1();
typedef void *onL2();
typedef void *onL3();
typedef void *onR1();
typedef void *onR2();
typedef void *onR3();

typedef struct {
	onStart *onPressStart;
	onSelect *onPressSelect;

	onCross *onPressCross;
	onCircle *onPressCircle;
	onTriangle *onPressTriangle;
	onSquare *onPressSquare;

	onLeft *onPressLeft;
	onRight *onPressRight;
	onUp *onPressUp;
	onDown *onPressDown;

	onL1 *onPressL1;
	onL2 *onPressL2;
	onL3 *onPressL3;
	onR1 *onPressR1;
	onR2 *onPressR2;
	onR3 *onPressR3;
} control_map;

#endif /* CONTROLLERMAPPING_H_ */
