#ifndef __engine_version_h
#define __engine_version_h

#ifdef JAVASCRIPT
const char* ENGINE_NAME = 0;
const char* ENGINE_VERSION = 0;
#else
const char* ENGINE_NAME = "kmdy-engine";
const char* ENGINE_VERSION = "0.0.0 (alpha test)";
#endif

#endif