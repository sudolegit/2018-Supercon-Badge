////////////////////////////////////////////////////////////////////////////////////////////////////
//!	@file
//!	
//!	@brief			Implements Wii communication by utilizing the external 3rd party Wii library 
//!					(and associated dependency libs).
////////////////////////////////////////////////////////////////////////////////////////////////////


//==================================================================================================
//	INCLUDES
//--------------------------------------------------------------------------------------------------
#include "hw.h"
#include "wii_lib.h"
#include "wii_interface.h"




//==================================================================================================
//	GLOBAL VARIABLES
//--------------------------------------------------------------------------------------------------
WiiLib_Device		m_WiiDevice;												// Instance of Wii target that is located on I2C bus.




//==================================================================================================
//	PRIVATE FUNCTION PROTOTYPES
//--------------------------------------------------------------------------------------------------
static void		WiiInterface_ProcessNunchuck(	uint8_t *key	);




//==================================================================================================
//	PUBLIC FUNCTIONS
//--------------------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////////////////
//!	@brief			Handle initialization and polling of Wii target devices. The state of the Wii 
//!					target is used to [potentially] override the provided key value.
//!	
//!	@details		Interfaces with 3rd party 'lib-wii' and handles initialization and polling of 
//!					the externally connected target Wii device. When successfully polled, invokes 
//!					the relevant processing function to handle any potential overriding of the 
//!					provided key value.
//!	
//!	@note			This function is invoked each time Timer-5 fires. That timer loop is used to 
//!					control polling of the keyboard as well as refreshing of the screen. We need 
//!					to throttle how often we interact with the Wii target to avoid issues. 
//!					Presently, Timer-5 runs at ~8-12 ms tick. Looking to delay roughly 50-60 ms 
//!					so the 'throttle' is set to '5' for 'WII_INTERFACE_THROTTLE_COUNT'.
//!	
//!	@param[in,out]	*key				Current key press determined by core of project that may 
//!										be overridden. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void WiiInterface_Refresh(uint8_t *key)
{
	static uint8_t		throttle		= WII_INTERFACE_THROTTLE_COUNT;
	
	if( !(--throttle) )
	{
		if( m_WiiDevice.status == WII_LIB_DEVICE_STATUS_STRUCTURE_NOT_DEFINED )
		{
			WiiLib_Init( I2C1, SYS_CLK, WII_LIB_TARGET_DEVICE_NUNCHUCK, TRUE, &m_WiiDevice );
		}
		else
		{
			WiiLib_DoMaintenance(&m_WiiDevice);
			
			if( WiiLib_PollStatus(&m_WiiDevice) == WII_LIB_RC_SUCCESS )
			{
				switch( m_WiiDevice.target )
				{
					case WII_LIB_TARGET_DEVICE_NUNCHUCK:
						WiiInterface_ProcessNunchuck(key);
						break;
					
					case WII_LIB_TARGET_DEVICE_CLASSIC_CONTROLLER:
					case WII_LIB_TARGET_DEVICE_MOTION_PLUS:
					case WII_LIB_TARGET_DEVICE_MOTION_PLUS_PASS_NUNCHUCK:
					case WII_LIB_TARGET_DEVICE_MOTION_PLUS_PASS_CLASSIC:
						// Do nothing yet, but listing possible other devices here for reference.
						break;
				}
			}
		}
		
		throttle = WII_INTERFACE_THROTTLE_COUNT;
		
	}
}




//==================================================================================================
//	PRIVATE FUNCTIONS
//--------------------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////////////////
//!	@brief			Interprets the state of the (global) Wii target as if it is a Wii Nunchuck 
//!					device and overrides the provided key value appropriately.
//!	
//!	@param[in,out]	*key				Current key press determined by core of project that may 
//!										be overridden. 
////////////////////////////////////////////////////////////////////////////////////////////////////
static void WiiInterface_ProcessNunchuck(uint8_t *key)
{
	// Add functions for enabling and disabling ignore of repeated key-press values.
	// Add support for key press states. Don't forget to find new home position when z pressed.
	*key = 'B';
}

