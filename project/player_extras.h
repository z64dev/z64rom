//
// player_extras.h
//
// this file exists as a portable alternative to
// modifying your z64hdr installation directly
//
// also do not use #include here, as this file is
// strictly for declaring types and variables
//
// lastly, you do not have to #include <player_extras.h>
// anywhere in order to access the types you define here
// (you get them automatically through z64hdr or uLib)
//








#ifdef Z64ROM_WANT_PLAYER_EXTRAS_TYPES_H /////////////////////////////////////
#undef Z64ROM_WANT_PLAYER_EXTRAS_TYPES_H /////////////////////////////////////
/*////////////////////////////////////////////////////////////////////////////

    ██
    ██    ██    ██  ██████
  ██████  ██    ██  ████████      ██        ██
  ██████  ████  ██  ██    ██    ██████    ████
    ██      ██████  ██    ██  ████  ██  ████
    ██        ████  ██  ████  ██  ████    ████
    ██      ██████  ██████    ██████        ████
    ██    ██████    ████      ██          ██████
    ████  ████      ██        ████████  ██████
      ██  ██        ██          ██████  ████
//
//
// any datatypes defined in this section
// can be used as types in the next one
//
// you can also use all types from z64hdr
*/

	typedef struct tagMyCustomPlayerTypeExample
	{
		Vec3s hello;
		Vec3s world;
	} MyCustomPlayerType;








//////////////////////////////////////////////////////////////////////////////
#else ////////////////////////////////////////////////////////////////////////
/*////////////////////////////////////////////////////////////////////////////

                              ██            ██        ██
                              ██            ██        ██
  ██    ██  ██████    ██  ██      ██████    ██        ██        ██        ██
  ██    ██  ████████  ██████  ██  ████████  ██████    ██      ██████    ████
  ██    ██        ██  ████    ██        ██  ████████  ██    ████  ██  ████
  ██    ██    ██████  ██      ██    ██████  ██    ██  ██    ██  ████    ████
  ██  ████  ████  ██  ██      ██  ████  ██  ██    ██  ██    ██████        ████
  ████████  ██    ██  ██      ██  ██    ██  ██  ████  ██    ██          ██████
  ██████    ████████  ██      ██  ████████  ██████    ████  ████████  ██████
    ██        ██████  ██      ██    ██████  ████        ██    ██████  ████


//
//
// any variables declared in this section will be
// added as Player struct members during compilation
//
*/

	MyCustomPlayerType fancy;
	int someCounter; // you could access this using player->someCounter








////////////////////////////////////////////////////////////////////////////
//////////////////// do not insert code below this line ////////////////////
////////////////////////////////////////////////////////////////////////////

#endif

