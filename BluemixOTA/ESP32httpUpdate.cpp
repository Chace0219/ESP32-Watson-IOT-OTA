/**
 *

 */

#include "ESP32httpUpdate.h"
#include <StreamString.h>

extern "C" uint32_t _SPIFFS_start;
extern "C" uint32_t _SPIFFS_end;

extern String X_AuthToken;

ESP32HTTPUpdate::ESP32HTTPUpdate(void)
{
}

ESP32HTTPUpdate::~ESP32HTTPUpdate(void)
{
}

HTTPUpdateResult ESP32HTTPUpdate::update(const String& url, const String& currentVersion,
	const String& CARoot, bool reboot)
{
    rebootOnUpdate(reboot);
	if (CARoot.length() == 0) {
		return update(url, currentVersion);
	}
	else {
		return update(url, currentVersion, CARoot);
	}
}

HTTPUpdateResult ESP32HTTPUpdate::update(const String& url, const String& currentVersion)
{
    HTTPClient http;
    http.begin(url);
    return handleUpdate(http, currentVersion, false);
}

HTTPUpdateResult ESP32HTTPUpdate::update(const String& url, const String& currentVersion,
	const String& CARoot)
{
    HTTPClient http;
    http.begin(url, CARoot);
	return handleUpdate(http, currentVersion, false);
}

HTTPUpdateResult ESP32HTTPUpdate::updateSpiffs(const String& url, const String& currentVersion, const String& CARoot)
{
    HTTPClient http;
    http.begin(url, CARoot);
    return handleUpdate(http, currentVersion, true);
}

HTTPUpdateResult ESP32HTTPUpdate::updateSpiffs(const String& url, const String& currentVersion)
{
    HTTPClient http;
    http.begin(url);
    return handleUpdate(http, currentVersion, true);
}

HTTPUpdateResult ESP32HTTPUpdate::update(const String& host, uint16_t port, const String& uri, const String& currentVersion,
        const String& CARoot, bool reboot)
{
    rebootOnUpdate(reboot);

	if (CARoot.length() == 0) {
        return update(host, port, uri, currentVersion);
    } else {
        return update(host, port, uri, currentVersion, CARoot);
    }
}

HTTPUpdateResult ESP32HTTPUpdate::update(const String& host, uint16_t port, const String& uri,
        const String& currentVersion)
{
    HTTPClient http;
    http.begin(host, port, uri);
    return handleUpdate(http, currentVersion, false);
}

HTTPUpdateResult ESP32HTTPUpdate::update(const String& host, uint16_t port, const String& uri,
        const String& currentVersion, const String& CARoot)
{
	HTTPClient http;
	http.begin(host, port, uri, CARoot);

	return handleUpdate(http, currentVersion, false);
}

/**
 * return error code as int
 * @return int error code
 */
int ESP32HTTPUpdate::getLastError(void)
{
    return _lastError;
}

/**
 * return error code as String
 * @return String error
 */
String ESP32HTTPUpdate::getLastErrorString(void)
{

    if(_lastError == 0) {
        return String(); // no error
    }

    // error from Update class
    if(_lastError > 0) {
        StreamString error;
        Update.printError(error);
        error.trim(); // remove line ending
        return String(F("Update error: ")) + error;
    }

    // error from http client
    if(_lastError > -100) {
        return String(F("HTTP error: ")) + HTTPClient::errorToString(_lastError);
    }

    switch(_lastError) {
    case HTTP_UE_TOO_LESS_SPACE:
        return F("To less space");
    case HTTP_UE_SERVER_NOT_REPORT_SIZE:
        return F("Server not Report Size");
    case HTTP_UE_SERVER_FILE_NOT_FOUND:
        return F("File not Found (404)");
    case HTTP_UE_SERVER_FORBIDDEN:
        return F("Forbidden (403)");
    case HTTP_UE_SERVER_WRONG_HTTP_CODE:
        return F("Wrong HTTP code");
    case HTTP_UE_SERVER_FAULTY_MD5:
        return F("Faulty MD5");
    case HTTP_UE_BIN_VERIFY_HEADER_FAILED:
        return F("Verify bin header failed");
    case HTTP_UE_BIN_FOR_WRONG_FLASH:
        return F("bin for wrong flash size");
    }

    return String();
}


/**
 *
 * @param http HTTPClient *
 * @param currentVersion const char *
 * @return HTTPUpdateResult
 */
HTTPUpdateResult ESP32HTTPUpdate::handleUpdate(HTTPClient& http, const String& currentVersion, bool spiffs)
{
	// 
    HTTPUpdateResult ret = HTTP_UPDATE_FAILED;

    // use HTTP/1.0 for update since the update handler not support any transfer Encoding
	http.useHTTP10(true);
	http.setTimeout(30000);

	//http.addHeader(F("X-Auth-Token"), X_AuthToken);

	delay(500);
	int code = http.GET();
	int len = http.getSize();
	Serial.print(F("HTTP Response Length="));
	Serial.println(len);

	if (code <= 0) {
		DEBUG_HTTP_UPDATE("[httpUpdate] HTTP error: %s\n", http.errorToString(code).c_str());
		_lastError = code;
		http.end();
		return HTTP_UPDATE_FAILED;
	}

	DEBUG_HTTP_UPDATE("[httpUpdate] Header read fin.\n");
	DEBUG_HTTP_UPDATE("[httpUpdate] Server header:\n");
	DEBUG_HTTP_UPDATE("[httpUpdate]  - code: %d\n", code);
	DEBUG_HTTP_UPDATE("[httpUpdate]  - len: %d\n", len);

	DEBUG_HTTP_UPDATE("[httpUpdate] ESP32 info:\n");

	if (currentVersion && currentVersion[0] != 0x00) 
	{
		DEBUG_HTTP_UPDATE("[httpUpdate]  - current version: %s\n", currentVersion.c_str());
	}

	switch (code) 
	{
	case HTTP_CODE_OK:  ///< OK (Start Update)
		{
			if (len > 0) 
			{
				if (!Update.begin(len, 0))
				{
					Serial.println("Update begin failed!");
					_lastError = Update.getError();
					DEBUG_HTTP_UPDATE("[httpUpdate] Update.begin failed! (%s)\n", error.c_str());
				}
				else
				{
					Serial.println("Update Started!");
					WiFiClientSecure * tcp = http.getStreamPtr();

					delay(100);

					int command;
					uint32_t written = 0, total = 0, tried = 0, Currwritten = 0;

					while (tcp->connected() && (len > 0 || len == -1))
					{
						size_t waited = 80000; // timeout
						size_t available = tcp->available();

						while (!available && waited)
						{
							delay(1);
							waited -= 1;
							available = tcp->available();
						}

						if (!available)
						{
							Serial.println("Timeout happens!!!");
							break;
						}

						tried = 0;
						static uint8_t buf[1460];
						if (available > 1460) 
						{
							available = 1460;
						}
						size_t r = tcp->read(buf, available);

						if (r != available) {
							log_w("didn't read enough! %u != %u", r, available);
						}
						else
						{
							len -= available;
							written = Update.write(buf, r);
							if (written > 0)
							{
								if (written != r) {
									log_w("didn't write enough! %u != %u", written, r);
								}
								total += written;
								Serial.print(total);
								Serial.println(" bytes written...");
							}
							else
							{
							}
						}

					}
					if (Update.end())
					{
						delay(100);
						Serial.print("Written Length is ");
						Serial.println(total);
					}

					if (_rebootOnUpdate)
					{
						Serial.println("Rebooting...");
						delay(500);
						ESP.restart();
					}

					ret = HTTP_UPDATE_OK;
					DEBUG_HTTP_UPDATE("[httpUpdate] Update ok\n");
				}


				http.end();

			}
			else 
			{
				_lastError = HTTP_UE_SERVER_NOT_REPORT_SIZE;
				ret = HTTP_UPDATE_FAILED;
				DEBUG_HTTP_UPDATE("[httpUpdate] Content-Length is 0 or not set by Server?!\n");
			}
		}
		break;
	case HTTP_CODE_NOT_MODIFIED:
		///< Not Modified (No updates)
		ret = HTTP_UPDATE_NO_UPDATES;
		break;
	case HTTP_CODE_NOT_FOUND:
		_lastError = HTTP_UE_SERVER_FILE_NOT_FOUND;
		ret = HTTP_UPDATE_FAILED;
		break;
	case HTTP_CODE_FORBIDDEN:
		_lastError = HTTP_UE_SERVER_FORBIDDEN;
		ret = HTTP_UPDATE_FAILED;
		break;
	default:
		_lastError = HTTP_UE_SERVER_WRONG_HTTP_CODE;
		ret = HTTP_UPDATE_FAILED;
		DEBUG_HTTP_UPDATE("[httpUpdate] HTTP Code is (%d)\n", code);
		//http.writeToStream(&Serial1);
		break;
	}

	http.end();
    return ret;
}

/**
 * write Update to flash
 * @param in Stream&
 * @param size uint32_t
 * @param md5 String
 * @return true if Update ok
 */
bool ESP32HTTPUpdate::runUpdate(Stream& in, uint32_t size, String md5, int command)
{
    StreamString error;

	Serial.println("Update is started!");
	Serial.print("size=");
	Serial.println(size);

    if(!Update.begin(size, command)) 
	{
		Serial.println("Update begin failed!");
		_lastError = Update.getError();
        Update.printError(error);
        error.trim(); // remove line ending
        DEBUG_HTTP_UPDATE("[httpUpdate] Update.begin failed! (%s)\n", error.c_str());
        return false;
    }

	Serial.println("Update begin Okay!");
	
	if(md5.length()) {
        if(!Update.setMD5(md5.c_str())) {
            _lastError = HTTP_UE_SERVER_FAULTY_MD5;
            DEBUG_HTTP_UPDATE("[httpUpdate] Update.setMD5 failed! (%s)\n", md5.c_str());
            return false;
        }
    }

    if(Update.writeStream(in) != size) 
	{
		Serial.println("Stream error!");

		_lastError = Update.getError();
        Update.printError(error);
        error.trim(); // remove line ending
        DEBUG_HTTP_UPDATE("[httpUpdate] Update.writeStream failed! (%s)\n", error.c_str());
        return false;
    }

	Serial.println("Update End!");

    if(!Update.end()) {
		Serial.println("Update End Error!");

        _lastError = Update.getError();
        Update.printError(error);
        error.trim(); // remove line ending
        DEBUG_HTTP_UPDATE("[httpUpdate] Update.end failed! (%s)\n", error.c_str());
        return false;
    }

    return true;
}



ESP32HTTPUpdate ESPhttpUpdate;
