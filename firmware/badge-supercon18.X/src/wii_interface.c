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
WiiLib_Device		m_WiiDevice;																	// Instance of Wii target that is located on I2C bus.
uint8_t				m_flagRepeatKeys	= WII_INTERFACE_DEFAULT_SEND_REPEAT_KEYS;					// Flag used to determine if repeated key presses on a target device should be ignored or utilized (TRUE == send repeated keys).
uint8_t				m_flagExitToMenu	= FALSE;													// Flag indicating that we should exit any existing function and return to the main menu.




//==================================================================================================
//	PRIVATE FUNCTION PROTOTYPES
//--------------------------------------------------------------------------------------------------
static void		WiiInterface_ProcessNunchuck(			uint8_t *key	);
static void		WiiInterface_ProcessClassicController(	uint8_t *key	);
static void		WiiInterface_EnableRepeatedKeys(		void			);
static void		WiiInterface_DisableRepeatedKeys(		void			);




//==================================================================================================
//	PUBLIC FUNCTIONS
//--------------------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////////////////
//!	@brief			Returns the present value for the flag used to track if we should bail out of a 
//!					program and back to the main menu.
////////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t WiiInterface_ExitToMenu( void )
{
	return m_flagExitToMenu;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//!	@brief			Enables flag used to track if we should bail out of a program and back to the 
//!					main menu.
////////////////////////////////////////////////////////////////////////////////////////////////////
void WiiInterface_EnableExitToMenu( void )
{
	m_flagExitToMenu = TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//!	@brief			Disables flag used to track if we should bail out of a program and back to the 
//!					main menu.
////////////////////////////////////////////////////////////////////////////////////////////////////
void WiiInterface_DisableExitToMenu( void )
{
	m_flagExitToMenu = FALSE;
}


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
//!					Presently, Timer-5 runs at ~8-12 ms tick. Looking to delay roughly 50 or 100 
//!					ms so the 'throttle' is set to '5' or '10' based on if target is a nunchuck or 
///!				a classic controller.
//!	
//!	@param[in,out]	*key				Current key press determined by core of project that may 
//!										be overridden. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void WiiInterface_Refresh(uint8_t *key)
{
	static uint8_t		throttle		= WII_INTERFACE_THROTTLE_COUNT_NUNCHUCK;
	
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
						WiiInterface_ProcessClassicController(key);
						break;
					
					case WII_LIB_TARGET_DEVICE_MOTION_PLUS:
					case WII_LIB_TARGET_DEVICE_MOTION_PLUS_PASS_NUNCHUCK:
					case WII_LIB_TARGET_DEVICE_MOTION_PLUS_PASS_CLASSIC:
						// Do nothing yet, but listing possible other devices here for reference.
						break;
				}
			}
		}
		
		if( m_WiiDevice.target == WII_LIB_TARGET_DEVICE_CLASSIC_CONTROLLER )
		{
			throttle = WII_INTERFACE_THROTTLE_COUNT_CLASSIC_CONTTROLLER;
		}
		else
		{
			throttle = WII_INTERFACE_THROTTLE_COUNT_NUNCHUCK;
		}
	}
}




//==================================================================================================
//	PRIVATE FUNCTIONS
//--------------------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////////////////
//!	@brief			Enables sending the same key value multiple times.
////////////////////////////////////////////////////////////////////////////////////////////////////
static void WiiInterface_EnableRepeatedKeys( void )
{
	m_flagRepeatKeys = TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//!	@brief			Disables sending the same key value multiple times.
////////////////////////////////////////////////////////////////////////////////////////////////////
static void WiiInterface_DisableRepeatedKeys( void )
{
	m_flagRepeatKeys = FALSE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//!	@brief			Interprets the state of the (global) Wii target as if it is a Wii Nunchuck 
//!					device and overrides the provided key value appropriately.
//!	
//!	@param[in,out]	*key				Current key press determined by core of project that may 
//!										be overridden. 
////////////////////////////////////////////////////////////////////////////////////////////////////
static void WiiInterface_ProcessNunchuck(uint8_t *key)
{
	static uint8_t		flagAccelActive		= FALSE;
	static uint8_t		prevKey				= 0;
	uint8_t				tmpKey				= 0;
	
	// Do actions based on c-button state.
	if( m_WiiDevice.interfaceCurrent.buttonC )
	{
		// Toggle repeated keys.
		if( m_WiiDevice.interfaceRelative.analogLeftY > WII_NUNCHUCK_THRESHOLD_ANALOG )
		{
			if( m_flagRepeatKeys )
				WiiInterface_DisableRepeatedKeys();
			else
				WiiInterface_EnableRepeatedKeys();
		}
		// Exit back to main menu
		else if( m_WiiDevice.interfaceRelative.analogLeftY < -WII_NUNCHUCK_THRESHOLD_ANALOG )
		{
			WiiInterface_EnableExitToMenu();
		}
		// Hit enter
		else
		{
			tmpKey = K_ENT;
		}
		
		flagAccelActive = FALSE;
		
	}
	// Utilize accelerometers for position movement:
	else if( m_WiiDevice.interfaceCurrent.buttonZL )
	{
		if( flagAccelActive == FALSE )
		{
			WiiLib_SaveCurrentPositionAsHome( &m_WiiDevice );
			flagAccelActive = TRUE;
		}
		else
		{
			if( abs(m_WiiDevice.interfaceRelative.accelX) > abs(m_WiiDevice.interfaceRelative.accelY) )
			{
				if(	m_WiiDevice.interfaceRelative.accelX < -WII_NUNCHUCK_THRESHOLD_ACCELEROMETER )
					tmpKey = K_LT;
				else if( m_WiiDevice.interfaceRelative.accelX > WII_NUNCHUCK_THRESHOLD_ACCELEROMETER )
					tmpKey = K_RT;
			}
			else
			{
				if( m_WiiDevice.interfaceRelative.accelY > WII_NUNCHUCK_THRESHOLD_ACCELEROMETER )
					tmpKey = K_DN;
				else if( m_WiiDevice.interfaceRelative.accelY < -WII_NUNCHUCK_THRESHOLD_ACCELEROMETER )
					tmpKey = K_UP;
			}
		}
	}
	// Utilize analog joystick for position movement.
	else
	{
		if( abs(m_WiiDevice.interfaceRelative.analogLeftX) > abs(m_WiiDevice.interfaceRelative.analogLeftY) )
		{
			if(	m_WiiDevice.interfaceRelative.analogLeftX < -WII_NUNCHUCK_THRESHOLD_ANALOG )
				tmpKey = K_LT;
			else if( m_WiiDevice.interfaceRelative.analogLeftX > WII_NUNCHUCK_THRESHOLD_ANALOG )
				tmpKey = K_RT;
		}
		else
		{
			if( m_WiiDevice.interfaceRelative.analogLeftY < -WII_NUNCHUCK_THRESHOLD_ANALOG )
				tmpKey = K_DN;
			else if( m_WiiDevice.interfaceRelative.analogLeftY > WII_NUNCHUCK_THRESHOLD_ANALOG )
				tmpKey = K_UP;
		}
		
		flagAccelActive = FALSE;
		
	}
	
	// Only override key value if repeating keys is allowed or the key is unique.
	if( tmpKey != 0 && (m_flagRepeatKeys || prevKey != tmpKey) )
		*key = tmpKey;
	
	prevKey = tmpKey;
	
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//!	@brief			Interprets the state of the (global) Wii target as if it is a Wii Classic 
//!					Controller device and overrides the provided key value appropriately.
//!	
//!	@param[in,out]	*key				Current key press determined by core of project that may 
//!										be overridden. 
////////////////////////////////////////////////////////////////////////////////////////////////////
static void WiiInterface_ProcessClassicController(uint8_t *key)
{
	static uint8_t		prevKey				= 0;
	uint8_t				tmpKey				= 0;
	
	// Toggle support for repeated keys:
	if( m_WiiDevice.interfaceCurrent.buttonX || m_WiiDevice.interfaceCurrent.buttonY )
		WiiInterface_EnableRepeatedKeys();
	else
		WiiInterface_DisableRepeatedKeys();
	
	// Exit back to main menu:
	if( m_WiiDevice.interfaceCurrent.buttonHome )
		WiiInterface_EnableExitToMenu();
	// Hit Enter:
	else if( m_WiiDevice.interfaceCurrent.buttonA || m_WiiDevice.interfaceCurrent.buttonB )
		tmpKey = K_ENT;
	// Handle navigation keys:
	else if( m_WiiDevice.interfaceCurrent.dpadDown )
		tmpKey = K_DN;
	else if( m_WiiDevice.interfaceCurrent.dpadLeft )
		tmpKey = K_LT;
	else if( m_WiiDevice.interfaceCurrent.dpadRight )
		tmpKey = K_RT;
	else if( m_WiiDevice.interfaceCurrent.dpadUp )
		tmpKey = K_UP;
	
	// Only override key value if repeating keys is allowed or the key is unique.
	if( tmpKey != 0 && (m_flagRepeatKeys || prevKey != tmpKey) )
		*key = tmpKey;
	
	prevKey = tmpKey;
	
}

