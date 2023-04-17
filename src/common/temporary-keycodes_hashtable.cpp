
//==============================================================================
//
//   keycodes_hashtable.cpp
//
//==============================================================================
//  arsscriptum - made in quebec 2020 <guillaumeplante.qc@gmail.com>
//==============================================================================


#include "stdafx.h"
#include "keycodes_hashtable.h"
#include "myexception.h"

void KeyCodesHashTable::InitializeKeyCodes(){

	// code for the key event 
	LOG_TRACE("Hashtable::InitializeKeyCodes::Put", "add \"\" ");
	put("", "");



} // InitializeKeyCodes END




void KeyCodesHashTable::InitializeData(){
	LOG_TRACE("KeyCodesHashTable::InitializeData","InitializeKeyCodes");
	InitializeKeyCodes();
}

void KeyCodesHashTable::Reset(){
	LOG_TRACE("KeyCodesHashTable::Reset","Reset");
	Hashtable::Clear();
}

void KeyCodesHashTable::DumpContentToLogs(){
	unsigned int i = 0;
	for (auto& x: htmap) {
		LOG_TRACE("KeyCodesHashTable::DumpContentToLogs","[%d] %s = %s",i++,x.first,x.second);
	}
}

unsigned int KeyCodesHashTable::Count(){
	LOG_TRACE("KeyCodesHashTable::Count","return %d",Hashtable::Size());
	return Hashtable::Size();
}

bool KeyCodesHashTable::Find(const char *first){
	bool result = true;
	try{
		std::unordered_map<const void *, const void *>::const_iterator got = htmap.find (first);

		if ( got == htmap.end() ){
		  	result = false;
			LOG_TRACE("KeyCodesHashTable::Find", "Find(%s): %s.", first, result ? "FOUND" : "NOT FOUND");
		}else{
			LOG_TRACE("KeyCodesHashTable::Find", "Find(%s): %s. Values: %s=%s", first, result ? "FOUND" : "NOT FOUND", got->first, got->second);
		  	result = true;
		}

		
  	}catch (exception& e){
  		LOG_ERROR("KeyCodesHashTable::Find","Exception Occured. Details %s.",e.what());
  		result = false;
  	}
  	return result;	
}

bool KeyCodesHashTable::Test(){
	bool result = true;
	try{
		LOG_WARNING("KeyCodesHashTable::Test","Reset: flushing all default entries");
		Reset();
    	LOG_TRACE("KeyCodesHashTable::Test","CURRENTLY COUNT %d ENTRIES",Hashtable::Size());
    	LOG_WARNING("KeyCodesHashTable::Test","InitializeData: Initializing all default entries");
    	InitializeData();
    	LOG_TRACE("KeyCodesHashTable::Test","CURRENTLY COUNT %d ENTRIES",Hashtable::Size());

    	LOG_TRACE("KeyCodesHashTable::Test","Find(%s): %s.","Insert",Find("Insert")?"FOUND":"NOT FOUND");
    	LOG_TRACE("KeyCodesHashTable::Test","Find(%s): %s.","Delete",Find("Delete")?"FOUND":"NOT FOUND");
    	LOG_TRACE("KeyCodesHashTable::Test","Find(%s): %s.","InvalidKey",Find("InvalidKey")?"FOUND":"NOT FOUND");

    	LOG_TRACE("KeyCodesHashTable::Test","GetCode(%s): \"%s\"","Insert",GetCode("Insert"));
    	LOG_TRACE("KeyCodesHashTable::Test","GetCode(%s): \"%s\"","KeyEnd",GetCode("KeyEnd"));
  	}catch (exception& e){
  		LOG_ERROR("KeyCodesHashTable::Test","Exception Occured. Details %s.",e.what());
  		result = false;
  	}
  	return result;	
}

