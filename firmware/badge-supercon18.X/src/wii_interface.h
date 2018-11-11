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
#define	WII_INTERFACE_THROTTLE_COUNT_NUNCHUCK				5;					//!< Number of times to skip over call to function (see comments for 'WiiInterface_Refresh()').
#define	WII_INTERFACE_THROTTLE_COUNT_CLASSIC_CONTTROLLER	10;					//!< Number of times to skip over call to function (see comments for 'WiiInterface_Refresh()').




//==================================================================================================
//	PUBIC FUNCTION PROTOTYPES
//--------------------------------------------------------------------------------------------------
void	WiiInterface_Refresh(	uint8_t *key_temp	);


#endif	// __WII_INTERFACE_H__
