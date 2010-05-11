/*
Copyright (C) 2003, 2010 - Wolfire Games

This file is part of Lugaru.

Lugaru is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/*
 * Copyright (c) 2002 Lane Roathe. All rights reserved.
 *	Developed in cooperation with Freeverse, Inc.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Portions Copyright (c) 1999-2002 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 1.1 (the "License").  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON- INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 *
 * Modified: $Date: 2002/04/27 21:42:01 $
 * Revision: $Id: nsp_network.c,v 1.1 2002/04/27 21:42:01 lane Exp $
 */

/* NOTES:

	- This example is designed so that all networking code is in this one file. The purpose is to make
		it easier for a programmer to study the basic use of OpenPlay's NetSprocket API and not to
		demonstrate good programming style or OOP practices.

	- This file is not game independent (hey, that's what OpenPlay is about!), it simply helped me keep
		the example use of NetSprocket's more managable ... I hope :)

	- I wanted the host (ie, server) to also be a player, and found it was pretty easy to do; I simply
		made sure that I tracked whether I was a server or client (see the _server variable in main.h) and
		then when sending all players a message I had to modify the "to" portion of the header and
		resend it to the host. This means the host is sending itself messages, which works great.

*/

/* ----------------------------------------------------------- Includes */

#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#include "main.h"

#include "network.h"

/* ----------------------------------------------------------- Macro definitions */

enum
{
	kMessageType_Question = 256,
	kMessageType_Answer,

	kMessageType_Information
};

#define kPlayerType 1234	/* just defines a way to allow/reject players of different types */


/* ----------------------------------------------------------- Type definitions */

/* Questions are sent out in message packets from server to players */

/* NOTE	Message string is ZERO terminated!	*/
/*		This allows us to strcpy, strcmp, etc. on raw packet data	*/
/*		Meaning packet size = sizeof(packet_t) + strlen(string)	*/

typedef struct
{
	NSpMessageHeader  header;

	char		str[1];			/* message (question, info, etc) string */

}MessagePacket_t, *MessagePacketPtr;

/* Answers are sent from the players to the server in this packet */

typedef struct
{
	NSpMessageHeader  header;

	char		answer;			/* finally, the answer */

}AnswerPacket_t, *AnswerPacketPtr;

/* ----------------------------------------------------------- Local Variables */

/* name of this version of the app*/
char *GameNameStr = "OpenPlay NSp API Example 1";

/* Local vars used to handle our interaction with OpenPlay's NetSprockets API */

static NSpGameReference			_gameReference = NULL;		/* general game info */
static NSpProtocolListReference	_protocolListRef = NULL;	/* list of protocols we've started up */

static NMBoolean _approved;		/* after requesting to join a new game, this tracks whether or not we were approved */
static NMBoolean _response;		/* true if we get a response following a request to join a new game (else we time out */

/*--------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------*/
#if __MWERKS__
	#pragma mark -
	#pragma mark È Local Routines
	#pragma mark -
#endif


/*--------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------*/
#if __MWERKS__
	#pragma mark -
	#pragma mark È Global Routines
	#pragma mark -
#endif

unsigned char *GameC2PStr
(
	char *s,			/* source C string */
	unsigned char *d	/* destination buffer for Pascal string */
)
{
	int c = 0;

	if( s && d )	/* make sure we got got inputs */
	{
		while( *s )
			d[++c] = (unsigned char)*s++;	/* no blockmove, copy & get length in one shot */
	}
	*d = (unsigned char)c;

	return( d );
}

char *GameP2CStr
(
	unsigned char *s,	/* source Pascal string */
	char *d				/* buffer to hold converted C string */
)
{
	if( s && d )
	{
		memcpy( d, s+1, *s );	/* pretty simple */
		d[*s] = 0;
	}
	return( d );
}

/* ================================================================================
	Handle all of the network messages that are sent to this application

	 EXIT:	0 == no error, else error code
*/

void NetworkHandleMessage( void )
{
	NSpMessageHeader *mhp;
	char str[256];				/* buffer for converted PStrings */

	/* Ask NetSprocket to give us a message if any are waiting */

	mhp = NSpMessage_Get( _gameReference );
	if( mhp )
	{
		/* Print out the type of message received */

		switch(mhp -> what)
		{

/* --- Handle NetSprocket's generic messages */

			case kNSpJoinApproved:
				printf( "Approved!\n\n", str );
				fflush(stdout);
				_response = true;
				_approved = true;	/* tell our waiting loop that we were approved */
#if __USE_SIOUX__
				{
					NMErr err;

					err = NSpGame_GetInfo( _gameReference, &gameInfo );
					if( !err )
						SIOUXSetTitle( gameInfo.name );
				}
#endif
				break;
			
			case kNSpJoinDenied:
				GameP2CStr( ((NSpJoinDeniedMessage *)mhp)->reason, str );
				printf( "Denied!\n   Reason: %s\n\n", str );
				fflush(stdout);
				//GameOver();

				_response = true;
				_approved = false;	/* tell our waiting loop we were denied */
				break;

			case kNSpError:
				printf( "*** ERROR *** (reported value: %d)\n\n", ((NSpErrorMessage *)mhp)->error );
				fflush(stdout);

				_response = true;
				_approved = false;
				break;
			
			case kNSpGameTerminated:
				printf( "--- Host Terminated Game ---\n\n" );
				fflush(stdout);
				//GameOver();

				_response = true;
				_approved = false;
				break;

			case kNSpPlayerJoined:
			{
				NSpGameInfo gameInfo;
				char gamename[256];
				NMErr err;

				err = NSpGame_GetInfo( _gameReference, &gameInfo );
				if( !err )
					GameP2CStr( gameInfo.name, gamename );
				else
					strcpy( gamename, "ERRROR" );

				GameP2CStr( ((NSpPlayerJoinedMessage *)mhp)->playerInfo.name, str );
				printf( "===> Player %s joined game '%s', %d players now!\n\n",
						str, gamename, ((NSpPlayerJoinedMessage *)mhp)->playerCount );
				fflush(stdout);

				/* Lets go ahead and re-send the current question, as the new guy deserves a shot at it too */
				/*if( GameIsHost() )
				{
					//QuestionPtr theQuestion = GameGetCurrentQuestion();
					//if (theQuestion)
					//	NetworkSendQuestion(theQuestion->question);
				}*/
			}
			break;

			case kNSpPlayerLeft:
				GameP2CStr( ((NSpPlayerLeftMessage *)mhp)->playerName, str );
				printf( "===> Player, %s, Left game, leaving %d players!\n\n",
						str, ((NSpPlayerLeftMessage *)mhp)->playerCount );
				fflush(stdout);
				break;

			case kNSpHostChanged:
				printf( "===> ??? Host changed to player ID %d\n\n",
						((NSpHostChangedMessage *)mhp)->newHost );
				fflush(stdout);
				break;

			case kNSpGroupCreated:
				printf( "===> Player #%d created a new Group, ID %d\n\n",
						((NSpCreateGroupMessage *)mhp)->requestingPlayer, ((NSpCreateGroupMessage *)mhp)->groupID );
				fflush(stdout);
				break;

			case kNSpGroupDeleted:
				printf( "===> Player #%d deleted group #%d\n\n",
						((NSpDeleteGroupMessage *)mhp)->requestingPlayer, ((NSpDeleteGroupMessage *)mhp)->groupID );
				fflush(stdout);
				break;
			
			case kNSpPlayerAddedToGroup:
				printf( "===> Player %d was added to group %d\n\n",
						((NSpAddPlayerToGroupMessage *)mhp)->player, ((NSpAddPlayerToGroupMessage *)mhp)->group );
				fflush(stdout);
				break;

			case kNSpPlayerRemovedFromGroup:
				printf( "===> Player %d was removed from group %d\n\n",
						((NSpRemovePlayerFromGroupMessage *)mhp)->player, ((NSpRemovePlayerFromGroupMessage *)mhp)->group );
				fflush(stdout);
				break;

			case kNSpPlayerTypeChanged:
				printf( "===> Player %d changed to a new type, %d\n\n",
						((NSpPlayerTypeChangedMessage *)mhp)->player, ((NSpPlayerTypeChangedMessage *)mhp)->newType );
				break;


/* --- Handle our game specific messages */


			/* Got a message, see if it is correct or not and let everyone know the results */

			case kMessageType_Answer:
			{
				NSpPlayerInfoPtr pip;
				char cname[kNSpStr32Len];
				NMErr err;

				err = NSpPlayer_GetInfo( _gameReference, mhp->from, &pip );
				if( !err )
				{		
					GameP2CStr( pip->name, cname );

					NSpPlayer_ReleaseInfo( _gameReference, pip );
				}
				else
					strcpy( cname, "UNKOWN -- error!" );

				//sprintf( str, "Player #%d, %s, answered with '%c', which is %s", mhp->from, cname, ((AnswerPacketPtr)mhp)->answer,
					//GameCheckAnswer( ((AnswerPacketPtr)mhp)->answer ) ? "Correct!" : "WRONG!" );

				NetworkSendInformation( str );
			}
			break;

			/* allow game to do any special processing needed when a question arrives */

			case kMessageType_Question:
				//GameDisplayQuestion( ((MessagePacketPtr)mhp)->str );
				break;

			/* pretty simple, just display the info message */

			case kMessageType_Information:
				printf( "%s\n\n", ((MessagePacketPtr)mhp)->str );
				fflush(stdout);
				break;

			default:
				break;
		}

		/* Once done with it, release the message */

		NSpMessage_Release( _gameReference, mhp );
	}
}

/* ================================================================================
	Return the # of players in the current game

	 EXIT:	none
*/

NMUInt32 NetworkGetPlayerCount( void )
{
	NSpGameInfo gameInfo;
	NMErr err;

	err = NSpGame_GetInfo( _gameReference, &gameInfo );	/* player count is in the game info record */
	if( !err )
	{
		return( gameInfo.currentPlayers );
	}

	return( 0 );
}

/* ================================================================================
	Send an answer to the server

	 EXIT:	none
*/

NMErr NetworkSendAnswer
(
	char answer			/* the answer to send (just a char!) */
)
{
	AnswerPacket_t answerPacket;
	NMErr err;

	/* init the NetSprocket portion of the packet */

	NSpClearMessageHeader( &answerPacket.header );

	answerPacket.header.what = kMessageType_Answer;
	answerPacket.header.to = kNSpHostOnly;
	answerPacket.header.messageLen = sizeof(answerPacket);

	/* fill in the data section */

	answerPacket.answer = answer;

	/* tell NetSprocket to send the message */

	err = NSpMessage_Send( _gameReference, &answerPacket.header, kNSpSendFlag_Registered );
	if( err )
	{
		printf( "*** ERROR *** Unable to send answer packet, error # %d\n\n", err );
		fflush(stdout);
	}

	return( err );
}

/* ================================================================================
	Send a message to all players

	 EXIT:	none
*/

NMErr NetworkSendPlayerMessage
(
	const char *message,		/* ptr to message string to send */
	NMUInt32 messageType		/* type of message (question, info, etc. */
)
{
	MessagePacketPtr qpp;
	unsigned long messageLen, size;
	NMErr err;

	/* sanity checks */

	if( !message )
		return( kNSpInvalidParameterErr );

	/* get size of message string & total size of network packet */

	messageLen = strlen( message );
	size = sizeof(MessagePacket_t) + messageLen + 1;	/* header + num_chars + terminator */

	/* allocate the memory for the packet */

	qpp = (MessagePacketPtr)malloc( size );
	if( !qpp )
	{
		printf( " *** ERROR *** Unable to allocate message buffer!\n\n" );
		fflush(stdout);
		return( kNSpMemAllocationErr );
	}

	/* init the NetSprocket portion of the packet */

	NSpClearMessageHeader( &qpp->header );

	qpp->header.what = (NMSInt32)messageType;
	qpp->header.to = kNSpAllPlayers;
	qpp->header.messageLen = size;

	/* fill in the data section */

	strcpy( qpp->str, message );

	/* tell NetSprocket to send the message */

	err = NSpMessage_Send( _gameReference, &qpp->header, kNSpSendFlag_Registered );
	if( !err )
	{
		qpp->header.to = kNSpHostOnly;		/* allow host to play as well! */

		err = NSpMessage_Send( _gameReference, &qpp->header, kNSpSendFlag_Registered );
	}

	if( err )
	{
		printf( "*** ERROR *** Unable to send message packet, error # %d\n\n", err );
		fflush(stdout);
	}

	/* clean up after ourselves! */

	free( qpp );

	return( err );
}

/* ================================================================================
	Send a question to all players

	 EXIT:	none
*/

NMErr NetworkSendQuestion
(
	const char *question	/* ptr to question string to send */
)
{
	return( NetworkSendPlayerMessage( question, kMessageType_Question ) );
}

/* ================================================================================
	Send information to all players

	 EXIT:	none
*/

NMErr NetworkSendInformation
(
	const char *message		/* ptr to information string to send */
)
{
	return( NetworkSendPlayerMessage( message, kMessageType_Information ) );
}

#if __MWERKS__
	#pragma mark -
#endif

/* ================================================================================
	Initialize server networking

	 EXIT:	0 == no error, else error code
*/

NMErr NetworkStartServer
(
	NMUInt16 port,					/* port clients will connect on */
	NMUInt32 maxPlayers,			/* max # of players to allow into game */
	const unsigned char *gameName,	/* name of game (displayed to clients on connect) */
	const unsigned char *playerName	/* name of player on server computer */
)
{
	NSpProtocolReference protocolRef;
	NMErr err;

	/* Create a new protocol list to store our IP protocol reference in */

	err = NSpProtocolList_New( NULL, &_protocolListRef );
	if( !err )
	{
		/* Create a protocol reference for our IP connection, on our specified port w/default maxRTT and throughput */

		protocolRef = NSpProtocol_CreateIP( port, 0, 0 );
		if( protocolRef )
		{
			/* We got a good reference, append it to the list we created earlier */

			err = NSpProtocolList_Append( _protocolListRef, protocolRef );
			if( !err )
			{
				/* We got a protocol and it's in our reference list, now we can create the game
					to host, using the parms sent in and defaulting rest of the "unused" hosting parms */

				err = NSpGame_Host( &_gameReference, _protocolListRef, maxPlayers, gameName,
									"\pPassword", playerName, kPlayerType, kNSpClientServer, 0 );
			}
		}
		else
			err = kNSpInvalidProtocolRefErr;	/* assign somewhat meaningful error on failure to create protocol reference */
	}

	return( err );
}


/* ================================================================================
	Shutdown the networking, release resources, etc.

	 EXIT:	0 == no error, else error code
*/

NMErr NetworkStartClient
(
	char *ipAddr,					/* IP address (or domain name) to look for server (host) on */
	char *port,						/* Port to talk to server via */
	const unsigned char *playerName	/* name of player wanting to join */
)
{
	NSpAddressReference addRef;
	NMErr err;

	/* Given our input strings, create an OpenPlay address reference for talking to server */

	addRef = NSpCreateIPAddressReference( ipAddr, port );
	if( addRef )
	{
		printf( "\nAttempting to join game..." );
		fflush(stdout);

		/* Now, look for a server on the IP/Port given and see if we can connect */

		err = NSpGame_Join( &_gameReference, addRef, playerName, "\pPassword", kPlayerType, NULL, 0, 0 );
		if( !err )
		{
			NMUInt32 startTime, currentTime;
			time_t seconds;

			printf( "connected!\n\nWaiting for approval to join game (press 'q' to quit)..." );
			fflush(stdout);

			time(&seconds);
			startTime = seconds;

			_response = _approved = false;

			/* We connected, now we have to wait for the server to approve our join request */

			while( !_response )
			{
				/* Check for a time out in connecting to server */
				/* this is before the event handler so that we are not approved, but time out anyway (small chance, but...) */

				time(&seconds);
				currentTime = seconds;

				if( (currentTime - startTime > 60) && (!_response) )
				{
					printf( "ERROR: Connection timed out!\n\n" );
					fflush(stdout);

					_response = true;
					_approved = false;
				}

				/* Handle system messages and allow user to quit via 'q' */
				/* This also gets and handles network messages, like accept/deny */

				//GameHandleEvents();
			}

			/* if we were not approved, we must dispose of the game object here */

			if( !_approved )
			{
				err = NSpGame_Dispose( _gameReference, kNSpGameFlag_ForceTerminateGame );
				_gameReference = NULL;
			}
			else	/* let the user know that they were accepted to the game */
			{
				NSpGameInfo gameInfo;
				char str[256];

				err = NSpGame_GetInfo( _gameReference, &gameInfo );
				if( !err )
				{
					GameP2CStr( gameInfo.name, str );

					printf( "   Welcome to the game '%s', with %d players\n\n", str, (int)gameInfo.currentPlayers );
					fflush(stdout);
				}
			}
		}
	}
	else
		err = kNMParameterErr;

	return( err );
}


#if __MWERKS__
	#pragma mark -
#endif

/* ================================================================================
	Shutdown the networking, release resources, etc.

	 EXIT:	0 == no error, else error code
*/

NMErr NetworkShutdown( void )
{
	NMUInt32 refCount;
	NMErr err = kNMNoError;

	/* if we have a game object (we should!) dispose if it now */

	if( _gameReference )
		err = NSpGame_Dispose( _gameReference, kNSpGameFlag_ForceTerminateGame );

	/* dispose of our protocol references & the list containing them */

	if( _protocolListRef )
	{
		NSpProtocolReference pRef;

		refCount = NSpProtocolList_GetCount( _protocolListRef );	/* get number of protocols */

		while( refCount-- && !err )
		{
			pRef = NSpProtocolList_GetIndexedRef( _protocolListRef, refCount );	/* get currect reference */

			err = NSpProtocolList_RemoveIndexed( _protocolListRef, refCount );	/* then remove it from the list */

			/* now, we can dispose of the reference safely */
			NSpProtocol_Dispose( pRef );	/* this should have an NMErr return, but has a void return... */
		}

		/* once all of the protocols are disposed, we can dispose of the containing reference list */
		NSpProtocolList_Dispose( _protocolListRef );
	}


	/* Make sure we can't use old values */

	_protocolListRef = NULL;
	_gameReference = NULL;
	
	return( err );
}

/* ================================================================================
	Startup the networking system (NetSprockets in this case)

	 EXIT:	0 == no error, else error code
*/

NMErr NetworkStartup( void )
{
	NMErr err;

	/* First, make sure that NetSprockets is available (we weak linked to OpenPlayStubLib) */

	if( NULL == NSpInitialize )	/*|| NULL == ProtocolAcceptConnection )*/
		return( errModuleNotFound );

	/* Initialize NetSprockets, 0 == use defaults & NSe1 stands for "NetSprocket Example 1" 
			It is an identifier for games of our type on the network */

	err = NSpInitialize( 0, 0, 0, 'NSe1', 0 );

	return( err );
}
