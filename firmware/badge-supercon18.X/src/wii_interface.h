////////////////////////////////////////////////////////////////////////////////////////////////////
//!	@file
//!	
//!	@brief			Defines constants, types, function prototypes, etc. for use with the Wii 
//!					interface functionality.
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef	__WII_INTERFACE_H__
#define	__WII_INTERFACE_H__


//==================================================================================================
//	CONSTANTS
//--------------------------------------------------------------------------------------------------
#define	WII_INTERFACE_THROTTLE_COUNT_NUNCHUCK				5					//!< Number of times to skip over call to function (see comments for 'WiiInterface_Refresh()'). Equates to a ~100ms delay.
#define	WII_INTERFACE_THROTTLE_COUNT_CLASSIC_CONTTROLLER	10					//!< Number of times to skip over call to function (see comments for 'WiiInterface_Refresh()'). Equates to a  ~50ms delay.

#define	WII_INTERFACE_DEFAULT_SEND_REPEAT_KEYS				FALSE				//!< Default flag enabling or disabling sending of repeated key presses detected on target device. Value of TRUE == send repeated; value of FALSE == only send each key once.




//==================================================================================================
//	PUBIC FUNCTION PROTOTYPES
//--------------------------------------------------------------------------------------------------
void	WiiInterface_Refresh(	uint8_t *key_temp	);


#endif	// __WII_INTERFACE_H__
