//
//  StateMACROS.h
//  StateDebug
//
//  Created by Eric Ocean on Sat Jun 26 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#define BEGIN_TRANSITIONS
#define END_TRANSITIONS
#define ACTION
#define NO_ACTION
#define NEXT_STATE

#define STATE(state_var) ( state_variable_##state_var )

#define DECLARE_STATE_VAR(state_var) \
	static int state_variable_##state_var;

#define GO_HISTORY( state_var ) \
	GO_STATE( state_var##_history )

#define GO_STATE( state_var, new_state ) \
	do { \
		state_var##_##new_state##_impl(); \
	} while(0)

#define MAKE_HISTORY( state_var ) \
	state_var##_history() \
	{ \
		GO_STATE( state_var, /* get current history state of state_var from global data somehow */ ) \
	}
	
#define GO_TRANSIENT(state_var, transient_name) \
	do { \
		state_var##_##transient_name(); \
	} while (0)

#define DECLARE_STATE( state_var, state_name ) \
	static void \
	state_var##_##state_name##_impl(); \
\
	static void \
	state_var##_##state_name();

#define DECLARE_TRANSIENT( state_var, transient_name ) \
	static void \
	state_var##_##transient_name();

#define DECLARE_EXIT( state_var ) \
	static void \
	state_var##_##exit();

#define MAKE_TRANSIENT( state_var, transient_name ) \
	static void \
	state_var##_##transient_name()

#define EMPTY_TRANSIENT \
	BEGIN_TRANSIENT \
	END_TRANSIENT

#define BEGIN_TRANSIENT
#define END_TRANSIENT

#define MAKE_STATE( state_var, state_name ) \
	static void \
	state_var##_##state_name##_impl() \
	{ \
		STATE(state_var) = state_name; \
		state_var##_##state_name(); \
	} \
	static void \
	state_var##_##state_name()

#define EMPTY_STATE \
	BEGIN_STATE \
	END_STATE

#define BEGIN_STATE
#define END_STATE

#define GO_EXIT( state_var  ) \
	do { \
		state_var##_##exit(); \
	} while (0)

#define MAKE_EXIT( state_var ) \
	static void \
	state_var##_##exit()
			
#define BEGIN_EXIT
#define END_EXIT

#define RETURN( error ) \
	return error;

#define EVENT_RETURN( event_name, error ) \
	return error;

#define EMPTY_EVENT( event_name ) \
	BEGIN_EVENT( event_name ) \
	END_EVENT

#define BEGIN_EVENT( event_name )
#define END_EVENT