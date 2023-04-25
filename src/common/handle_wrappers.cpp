
//==============================================================================
//
//   handle_wrappers.cpp
//
//==============================================================================
//  Copyright (C) Guilaume Plante 2020 <guillaumeplante.qc@gmail.com>
//==============================================================================

#include "stdafx.h"
#include "handle_wrappers.h"

void INTERNAL::dialogboxcleanup(HWND h){
	if (h!=nullptr) EndDialog(h, 0); 
}
void INTERNAL::hwndtimercleanup(HWNDTimer* h){
	if (h != nullptr){
		KillTimer(h->h, h->id);
		delete h;
	}
}
void INTERNAL::socketcleanup(SOCKETWrapper* s){
	if (s->socket != INVALID_SOCKET){
		shutdown(s->socket, SD_SEND);//allow sends to go out.. stop receiving
		closesocket(s->socket);
	}
	delete s;
}
