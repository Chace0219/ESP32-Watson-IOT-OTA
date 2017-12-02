
/*
 * Modified By Jin zhoyun
 * Date 201704
 * WifiManagerBluemix.ino
 * This file contains the logic needed to launch the WifiManager, connect to the Wifi network and bluemix.
 * Upon successfully making connections to these services, setupSensors() in BluemixClient2.ino is called 
 * and control is passed to it. 
 */

#include <WiFi.h>
#include "PubSubClient.h" // https://github.com/knolleary/pubsubclient/releases/tag/v2.3
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson/releases/tag/v5.0.7
#include <TimeLib.h>
#include <Wire.h>
#include "ESP32httpUpdate.h"
#include "globals.h"
#include <WiFiClientSecure.h>
#include <Preferences.h>

const char* OTA_SERVER_ROOT_CA_PEM = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIHeTCCBmGgAwIBAgIQC/20CQrXteZAwwsWyVKaJzANBgkqhkiG9w0BAQsFADB1\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMTQwMgYDVQQDEytEaWdpQ2VydCBTSEEyIEV4dGVuZGVk\n" \
"IFZhbGlkYXRpb24gU2VydmVyIENBMB4XDTE2MDMxMDAwMDAwMFoXDTE4MDUxNzEy\n" \
"MDAwMFowgf0xHTAbBgNVBA8MFFByaXZhdGUgT3JnYW5pemF0aW9uMRMwEQYLKwYB\n" \
"BAGCNzwCAQMTAlVTMRkwFwYLKwYBBAGCNzwCAQITCERlbGF3YXJlMRAwDgYDVQQF\n" \
"Ewc1MTU3NTUwMSQwIgYDVQQJExs4OCBDb2xpbiBQIEtlbGx5LCBKciBTdHJlZXQx\n" \
"DjAMBgNVBBETBTk0MTA3MQswCQYDVQQGEwJVUzETMBEGA1UECBMKQ2FsaWZvcm5p\n" \
"YTEWMBQGA1UEBxMNU2FuIEZyYW5jaXNjbzEVMBMGA1UEChMMR2l0SHViLCBJbmMu\n" \
"MRMwEQYDVQQDEwpnaXRodWIuY29tMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIB\n" \
"CgKCAQEA54hc8pZclxgcupjiA/F/OZGRwm/ZlucoQGTNTKmBEgNsrn/mxhngWmPw\n" \
"bAvUaLP//T79Jc+1WXMpxMiz9PK6yZRRFuIo0d2bx423NA6hOL2RTtbnfs+y0PFS\n" \
"/YTpQSelTuq+Fuwts5v6aAweNyMcYD0HBybkkdosFoDccBNzJ92Ac8I5EVDUc3Or\n" \
"/4jSyZwzxu9kdmBlBzeHMvsqdH8SX9mNahXtXxRpwZnBiUjw36PgN+s9GLWGrafd\n" \
"02T0ux9Yzd5ezkMxukqEAQ7AKIIijvaWPAJbK/52XLhIy2vpGNylyni/DQD18bBP\n" \
"T+ZG1uv0QQP9LuY/joO+FKDOTler4wIDAQABo4IDejCCA3YwHwYDVR0jBBgwFoAU\n" \
"PdNQpdagre7zSmAKZdMh1Pj41g8wHQYDVR0OBBYEFIhcSGcZzKB2WS0RecO+oqyH\n" \
"IidbMCUGA1UdEQQeMByCCmdpdGh1Yi5jb22CDnd3dy5naXRodWIuY29tMA4GA1Ud\n" \
"DwEB/wQEAwIFoDAdBgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwdQYDVR0f\n" \
"BG4wbDA0oDKgMIYuaHR0cDovL2NybDMuZGlnaWNlcnQuY29tL3NoYTItZXYtc2Vy\n" \
"dmVyLWcxLmNybDA0oDKgMIYuaHR0cDovL2NybDQuZGlnaWNlcnQuY29tL3NoYTIt\n" \
"ZXYtc2VydmVyLWcxLmNybDBLBgNVHSAERDBCMDcGCWCGSAGG/WwCATAqMCgGCCsG\n" \
"AQUFBwIBFhxodHRwczovL3d3dy5kaWdpY2VydC5jb20vQ1BTMAcGBWeBDAEBMIGI\n" \
"BggrBgEFBQcBAQR8MHowJAYIKwYBBQUHMAGGGGh0dHA6Ly9vY3NwLmRpZ2ljZXJ0\n" \
"LmNvbTBSBggrBgEFBQcwAoZGaHR0cDovL2NhY2VydHMuZGlnaWNlcnQuY29tL0Rp\n" \
"Z2lDZXJ0U0hBMkV4dGVuZGVkVmFsaWRhdGlvblNlcnZlckNBLmNydDAMBgNVHRMB\n" \
"Af8EAjAAMIIBfwYKKwYBBAHWeQIEAgSCAW8EggFrAWkAdgCkuQmQtBhYFIe7E6LM\n" \
"Z3AKPDWYBPkb37jjd80OyA3cEAAAAVNhieoeAAAEAwBHMEUCIQCHHSEY/ROK2/sO\n" \
"ljbKaNEcKWz6BxHJNPOtjSyuVnSn4QIgJ6RqvYbSX1vKLeX7vpnOfCAfS2Y8lB5R\n" \
"NMwk6us2QiAAdgBo9pj4H2SCvjqM7rkoHUz8cVFdZ5PURNEKZ6y7T0/7xAAAAVNh\n" \
"iennAAAEAwBHMEUCIQDZpd5S+3to8k7lcDeWBhiJASiYTk2rNAT26lVaM3xhWwIg\n" \
"NUqrkIODZpRg+khhp8ag65B8mu0p4JUAmkRDbiYnRvYAdwBWFAaaL9fC7NP14b1E\n" \
"sj7HRna5vJkRXMDvlJhV1onQ3QAAAVNhieqZAAAEAwBIMEYCIQDnm3WStlvE99GC\n" \
"izSx+UGtGmQk2WTokoPgo1hfiv8zIAIhAPrYeXrBgseA9jUWWoB4IvmcZtshjXso\n" \
"nT8MIG1u1zF8MA0GCSqGSIb3DQEBCwUAA4IBAQCLbNtkxuspqycq8h1EpbmAX0wM\n" \
"5DoW7hM/FVdz4LJ3Kmftyk1yd8j/PSxRrAQN2Mr/frKeK8NE1cMji32mJbBqpWtK\n" \
"/+wC+avPplBUbNpzP53cuTMF/QssxItPGNP5/OT9Aj1BxA/NofWZKh4ufV7cz3pY\n" \
"RDS4BF+EEFQ4l5GY+yp4WJA/xSvYsTHWeWxRD1/nl62/Rd9FN2NkacRVozCxRVle\n" \
"FrBHTFxqIP6kDnxiLElBrZngtY07ietaYZVLQN/ETyqLQftsf8TecwTklbjvm8NT\n" \
"JqbaIVifYwqwNN+4lRxS3F5lNlA/il12IOgbRioLI62o8G0DaEUQgHNf8vSG\n" \
"-----END CERTIFICATE-----\n" \
"-----BEGIN CERTIFICATE-----\n" \
"MIIEtjCCA56gAwIBAgIQDHmpRLCMEZUgkmFf4msdgzANBgkqhkiG9w0BAQsFADBs\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSswKQYDVQQDEyJEaWdpQ2VydCBIaWdoIEFzc3VyYW5j\n" \
"ZSBFViBSb290IENBMB4XDTEzMTAyMjEyMDAwMFoXDTI4MTAyMjEyMDAwMFowdTEL\n" \
"MAkGA1UEBhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0IEluYzEZMBcGA1UECxMQd3d3\n" \
"LmRpZ2ljZXJ0LmNvbTE0MDIGA1UEAxMrRGlnaUNlcnQgU0hBMiBFeHRlbmRlZCBW\n" \
"YWxpZGF0aW9uIFNlcnZlciBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoC\n" \
"ggEBANdTpARR+JmmFkhLZyeqk0nQOe0MsLAAh/FnKIaFjI5j2ryxQDji0/XspQUY\n" \
"uD0+xZkXMuwYjPrxDKZkIYXLBxA0sFKIKx9om9KxjxKws9LniB8f7zh3VFNfgHk/\n" \
"LhqqqB5LKw2rt2O5Nbd9FLxZS99RStKh4gzikIKHaq7q12TWmFXo/a8aUGxUvBHy\n" \
"/Urynbt/DvTVvo4WiRJV2MBxNO723C3sxIclho3YIeSwTQyJ3DkmF93215SF2AQh\n" \
"cJ1vb/9cuhnhRctWVyh+HA1BV6q3uCe7seT6Ku8hI3UarS2bhjWMnHe1c63YlC3k\n" \
"8wyd7sFOYn4XwHGeLN7x+RAoGTMCAwEAAaOCAUkwggFFMBIGA1UdEwEB/wQIMAYB\n" \
"Af8CAQAwDgYDVR0PAQH/BAQDAgGGMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEF\n" \
"BQcDAjA0BggrBgEFBQcBAQQoMCYwJAYIKwYBBQUHMAGGGGh0dHA6Ly9vY3NwLmRp\n" \
"Z2ljZXJ0LmNvbTBLBgNVHR8ERDBCMECgPqA8hjpodHRwOi8vY3JsNC5kaWdpY2Vy\n" \
"dC5jb20vRGlnaUNlcnRIaWdoQXNzdXJhbmNlRVZSb290Q0EuY3JsMD0GA1UdIAQ2\n" \
"MDQwMgYEVR0gADAqMCgGCCsGAQUFBwIBFhxodHRwczovL3d3dy5kaWdpY2VydC5j\n" \
"b20vQ1BTMB0GA1UdDgQWBBQ901Cl1qCt7vNKYApl0yHU+PjWDzAfBgNVHSMEGDAW\n" \
"gBSxPsNpA/i/RwHUmCYaCALvY2QrwzANBgkqhkiG9w0BAQsFAAOCAQEAnbbQkIbh\n" \
"hgLtxaDwNBx0wY12zIYKqPBKikLWP8ipTa18CK3mtlC4ohpNiAexKSHc59rGPCHg\n" \
"4xFJcKx6HQGkyhE6V6t9VypAdP3THYUYUN9XR3WhfVUgLkc3UHKMf4Ib0mKPLQNa\n" \
"2sPIoc4sUqIAY+tzunHISScjl2SFnjgOrWNoPLpSgVh5oywM395t6zHyuqB8bPEs\n" \
"1OG9d4Q3A84ytciagRpKkk47RpqF/oOi+Z6Mo8wNXrM9zwR4jxQUezKcxwCmXMS1\n" \
"oVWNWlZopCJwqjyBcdmdqEU79OX2olHdx3ti6G8MdOu42vi/hw15UJGQmxg7kVkn\n" \
"8TUoE6smftX3eg==\n" \
"-----END CERTIFICATE-----\n";

const char* WATSONIOT_CA_CERT = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIFqzCCBJOgAwIBAgIQCxjQE+z7NGL2+H7H3H33EDANBgkqhkiG9w0BAQsFADBN\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMScwJQYDVQQDEx5E\n" \
"aWdpQ2VydCBTSEEyIFNlY3VyZSBTZXJ2ZXIgQ0EwHhcNMTQwOTIyMDAwMDAwWhcN\n" \
"MTcxMTI5MTIwMDAwWjCBwjELMAkGA1UEBhMCR0IxEjAQBgNVBAgTCUhhbXBzaGly\n" \
"ZTETMBEGA1UEBxMKV2luY2hlc3RlcjEuMCwGA1UEChMlSW50ZXJuYXRpb25hbCBC\n" \
"dXNpbmVzcyBNYWNoaW5lcyBDb3JwLjEmMCQGA1UECxMdSW50ZXJuZXQgb2YgVGhp\n" \
"bmdzIEZvdW5kYXRpb24xMjAwBgNVBAMMKSoubWVzc2FnaW5nLmludGVybmV0b2Z0\n" \
"aGluZ3MuaWJtY2xvdWQuY29tMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKC\n" \
"AQEAtkFprFEGn/3D4PpShvthi1RySonpvPL5S6/33rP9HH+pZRThnTRd9kTQWGQr\n" \
"zS+bpGKwqIX6G+FixY1G08CDa+/JejLT1BiGAXCYY54/OCg7WvBr8feZ9mnK9GtB\n" \
"XArF0u2TPdFTObXlE9ZBj5BDx9+5CS3Y0Grc9/Yg7S19JulhH37ZRxj1K0iCLJnU\n" \
"NWuDwmXtCAvv/nMQ00upmNX4po+Jfo17bqmyGrvZWdJpFquWQlC2lzUkQ3clzc9k\n" \
"wfu/mfHdgS55IloU4CwF+9nAkBSDhX8YQeJmZf3KDMIXoAhplrpAid221QNDKEfT\n" \
"oE5ZGJcN9zhdrb+ZFInu7w66eQIDAQABo4ICDzCCAgswHwYDVR0jBBgwFoAUD4Bh\n" \
"HIIxYdUvKOeNRji0LOHG2eIwHQYDVR0OBBYEFF6uJ2Mig7JY8gVIz2+cxymckEho\n" \
"MF0GA1UdEQRWMFSCKSoubWVzc2FnaW5nLmludGVybmV0b2Z0aGluZ3MuaWJtY2xv\n" \
"dWQuY29tgidtZXNzYWdpbmcuaW50ZXJuZXRvZnRoaW5ncy5pYm1jbG91ZC5jb20w\n" \
"DgYDVR0PAQH/BAQDAgWgMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjBr\n" \
"BgNVHR8EZDBiMC+gLaArhilodHRwOi8vY3JsMy5kaWdpY2VydC5jb20vc3NjYS1z\n" \
"aGEyLWcyLmNybDAvoC2gK4YpaHR0cDovL2NybDQuZGlnaWNlcnQuY29tL3NzY2Et\n" \
"c2hhMi1nMi5jcmwwQgYDVR0gBDswOTA3BglghkgBhv1sAQEwKjAoBggrBgEFBQcC\n" \
"ARYcaHR0cHM6Ly93d3cuZGlnaWNlcnQuY29tL0NQUzB8BggrBgEFBQcBAQRwMG4w\n" \
"JAYIKwYBBQUHMAGGGGh0dHA6Ly9vY3NwLmRpZ2ljZXJ0LmNvbTBGBggrBgEFBQcw\n" \
"AoY6aHR0cDovL2NhY2VydHMuZGlnaWNlcnQuY29tL0RpZ2lDZXJ0U0hBMlNlY3Vy\n" \
"ZVNlcnZlckNBLmNydDAMBgNVHRMBAf8EAjAAMA0GCSqGSIb3DQEBCwUAA4IBAQBE\n" \
"sTL/E2gWP8nWd5wor5sJpVD4nAnTluR2fSqoBnTi4DlxY+5ujpHYSQIo8f8rp1Fr\n" \
"ADzy63x3woCNZXtWf5fs1uAJUHi53HK5Y9UH6nFBv4qwnSUiH+8JE+xBWVGT6bl0\n" \
"uM8miFZVo7r4Jpuc1I13B9oHqxtP2ayQDyMvtcENOv9JvqZk9TEvp7IJgdtmYE4d\n" \
"BsbKsHRC0m3S1tfXac9cuKA1rND21buWis9nV0cEabwT5XM4YpSITVeIdII6kvZ8\n" \
"SREOxEG3urjy2xU8RxDDiMTcAYjNwSNshfZPyY7TijFDl4ZXpioHuQ+N3NxtZKzg\n" \
"51QXrsnRosOX+sa4iSXx\n" \
"-----END CERTIFICATE-----\n" \
"-----BEGIN CERTIFICATE-----\n" \
"MIIElDCCA3ygAwIBAgIQAf2j627KdciIQ4tyS8+8kTANBgkqhkiG9w0BAQsFADBh\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n" \
"QTAeFw0xMzAzMDgxMjAwMDBaFw0yMzAzMDgxMjAwMDBaME0xCzAJBgNVBAYTAlVT\n" \
"MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxJzAlBgNVBAMTHkRpZ2lDZXJ0IFNIQTIg\n" \
"U2VjdXJlIFNlcnZlciBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB\n" \
"ANyuWJBNwcQwFZA1W248ghX1LFy949v/cUP6ZCWA1O4Yok3wZtAKc24RmDYXZK83\n" \
"nf36QYSvx6+M/hpzTc8zl5CilodTgyu5pnVILR1WN3vaMTIa16yrBvSqXUu3R0bd\n" \
"KpPDkC55gIDvEwRqFDu1m5K+wgdlTvza/P96rtxcflUxDOg5B6TXvi/TC2rSsd9f\n" \
"/ld0Uzs1gN2ujkSYs58O09rg1/RrKatEp0tYhG2SS4HD2nOLEpdIkARFdRrdNzGX\n" \
"kujNVA075ME/OV4uuPNcfhCOhkEAjUVmR7ChZc6gqikJTvOX6+guqw9ypzAO+sf0\n" \
"/RR3w6RbKFfCs/mC/bdFWJsCAwEAAaOCAVowggFWMBIGA1UdEwEB/wQIMAYBAf8C\n" \
"AQAwDgYDVR0PAQH/BAQDAgGGMDQGCCsGAQUFBwEBBCgwJjAkBggrBgEFBQcwAYYY\n" \
"aHR0cDovL29jc3AuZGlnaWNlcnQuY29tMHsGA1UdHwR0MHIwN6A1oDOGMWh0dHA6\n" \
"Ly9jcmwzLmRpZ2ljZXJ0LmNvbS9EaWdpQ2VydEdsb2JhbFJvb3RDQS5jcmwwN6A1\n" \
"oDOGMWh0dHA6Ly9jcmw0LmRpZ2ljZXJ0LmNvbS9EaWdpQ2VydEdsb2JhbFJvb3RD\n" \
"QS5jcmwwPQYDVR0gBDYwNDAyBgRVHSAAMCowKAYIKwYBBQUHAgEWHGh0dHBzOi8v\n" \
"d3d3LmRpZ2ljZXJ0LmNvbS9DUFMwHQYDVR0OBBYEFA+AYRyCMWHVLyjnjUY4tCzh\n" \
"xtniMB8GA1UdIwQYMBaAFAPeUDVW0Uy7ZvCj4hsbw5eyPdFVMA0GCSqGSIb3DQEB\n" \
"CwUAA4IBAQAjPt9L0jFCpbZ+QlwaRMxp0Wi0XUvgBCFsS+JtzLHgl4+mUwnNqipl\n" \
"5TlPHoOlblyYoiQm5vuh7ZPHLgLGTUq/sELfeNqzqPlt/yGFUzZgTHbO7Djc1lGA\n" \
"8MXW5dRNJ2Srm8c+cftIl7gzbckTB+6WohsYFfZcTEDts8Ls/3HB40f/1LkAtDdC\n" \
"2iDJ6m6K7hQGrn2iWZiIqBtvLfTyyRRfJs8sjX7tN8Cp1Tm5gr8ZDOo0rwAhaPit\n" \
"c+LJMto4JQtV05od8GiG7S5BNO98pVAdvzr508EIDObtHopYJeS4d60tbvVS3bR0\n" \
"j6tJLp07kzQoH3jOlOrHvdPJbRzeXDLz\n" \
"-----END CERTIFICATE-----\n" \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n" \
"QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n" \
"MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n" \
"b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n" \
"9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n" \
"CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n" \
"nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n" \
"43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n" \
"T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n" \
"gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n" \
"BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n" \
"TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n" \
"DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n" \
"hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n" \
"06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n" \
"PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n" \
"YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n" \
"CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\n" \
"-----END CERTIFICATE-----\n";

String X_AuthToken;

Preferences preferences;

//Device management subscription callback
void callback(char* topic, byte* payload, unsigned int payloadLength); 

const char WiFiAPPSK[] = "mike201704";              //Wifi Password when the ESP sets up AP
String header = ""
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n\r\n"
                "<!DOCTYPE HTML>\r\n<html>\r\n";    //HTTP header for server (wifimanager)

char * server;                                      //IBM Watson server
char authMethod[] = "use-token-auth";               //Authentication method used to connect to IBM
char * token;                                       //Token used to connect to IBM
char * clientId;                                    //ClientID used to connect to IBM

WiFiClientSecure wifiClient;                        //Secure client used to connect to IBM
PubSubClient * client;                              //Wrapper for wificlient allowing use of MQTT

WiFiServer serverWeb(80);                           //Spins up the server that wifimanager uses on first startup
int configured = 0;                                 //Tracks configuration state of the ESP: 1 - credentials have been entered, 2 - credentials confirmed as correct
long strt = 0;                                      //Tracks the time elapsed since credentials were entered (only on first startup)
int waitInterval = 10000;                           //Time until the ESP restarts after receiving credentials

uint16_t IntervalTime = 30;

/*
 * checkSubscriptionTopics
 * Desc: Called periodically to ping IoTWatson and check for subscription messages
 * Input: void
 * Output: void
 */
void checkSubscriptionTopics(){
	if (!client->loop())  //Ping IoTWatson
	{
		mqttConnect();      //If unable to ping, check for valid connection
	}
}

/*
 * setupWifiServer
 * Desc: Used to prepare the ESP to serve up the credentials screen on first startup.
 * Input: void
 * Output: void
 */
void setupWifiServer() 
{
	WiFi.mode(WIFI_AP_STA);
	// Do a little work to get a unique-ish name. Append the
	// last two bytes of the MAC (HEX'd) to "mike":
	uint8_t mac[6];
	WiFi.softAPmacAddress(mac);
	String macID = String(mac[4], HEX) +
					String(mac[5], HEX);
	macID.toUpperCase();
	String AP_NameString = "mike_" + macID;
	char AP_Name[32]; 
	AP_NameString.toCharArray(AP_Name, AP_NameString.length() + 1);
	Serial.println(AP_Name);
	WiFi.softAP(AP_Name, WiFiAPPSK);
	serverWeb.begin();

	Serial.print(F("I have started WIFI Web server!!!"));
}

/*
 * setupWifi
 * Desc: Pulls the SSID and password from EEPROM and attempts to connect to the Wifi
 *       network (using a 60 sec timeout)
 * Input: void
 * Output: true is connected, otherwise false
 */
boolean setupWiFi()
{ 
	Serial.print(F("Connecting to Network: "));

	//Get the SSID and Password
	//String ssid_verified = buildStringLoc(SSID_STR);
	String ssid_verified = "TP-LINK_64FB80";

	Serial.println(ssid_verified);
	//WiFi.begin((const char *)ssid_verified.c_str(), (const char *)(buildStringLoc(PASSWORD)).c_str());
	WiFi.begin((const char *)ssid_verified.c_str(), "chunxing151201");

	//Attempt Wifi connection
	int timeout = 60;
	while (WiFi.status() != WL_CONNECTED && timeout > 0) 
	{
		delay(500);
		Serial.print(F("."));
		timeout --;
	} 
	
	if (timeout != 0) 
	{
		Serial.print(F("WiFi connected, IP address: ")); Serial.println(WiFi.localIP());
		return true;
	} 
	else 
		return false;
}

/*
 * setUpBluemix
 * Desc: Pulls the bluemix credentials out of EEPROM and prepares the PubSubClient to
 *       connect to Watson.
 * Input: void
 * Output: true if connected, otherwise false
 */
boolean setUpBluemix()
{
    //Collect credentials from Peferences
	String org = buildStringLoc(ORG);
	String device_type = buildStringLoc(DEVTYP);
	String device_id = buildStringLoc(DEVTID);
	String tken = buildStringLoc(TOKEN);

	// Selecet Certificate for WATSON MQTT
	wifiClient.setCACert(WATSONIOT_CA_CERT);
	
	org = "v2nbj9";
	device_type = "ESP32";
	device_id = "Test1";
	tken = "IcbVLnRt4XGjh!MA8e";

	//Configure credentials for connection to bluemix
	server = (char *)calloc(1, strlen(org.c_str()) + strlen(".messaging.internetofthings.ibmcloud.com") + 1);
	strcpy(server, org.c_str());
	strcat(server, ".messaging.internetofthings.ibmcloud.com");
	token = (char *) calloc(1, strlen(tken.c_str()) + 1);
	strcpy(token, tken.c_str());
	clientId = (char *)calloc(1, strlen("d:::") + strlen(org.c_str()) + strlen(device_type.c_str()) + strlen(device_id.c_str()) + 1);
	strcpy(clientId, "d:");
	strcat(clientId, org.c_str());
	strcat(clientId, ":");
	strcat(clientId, device_type.c_str());
	strcat(clientId, ":");
	strcat(clientId, device_id.c_str());

	if (client && client->connected())
	{
		client->disconnect();
	}
	delay(500);
	client = new PubSubClient(server, 8883, callback, wifiClient);

	return mqttConnect();
}

/*
 * mqttConnect
 * Desc: Used to connect to IoT Watson
 * Input: void
 * Output: true if connected, false if timeout
 */
boolean mqttConnect() 
{
	int timeout = 60;
	if (!!!client->connected()) 
	{  
		Serial.print(F("Reconnecting MQTT client to ")); Serial.println(server);
		while (!!!client->connect(clientId, authMethod, token) && timeout != 0) 
		{
			Serial.print(".");
			delay(1000);
			timeout --;
		}
		Serial.println();
	}
	if (timeout) 
		return true;
	return false;
}

/*
 * scanForSSID
 * Desc: This method is called when the ESP has been unable to connect to the Wifi network.
 *      In order to determine if the network is down, or something elese happened (e.g. password changed),
 *      the ESP will scan to see if the desired SSID is present.
 * Input: void 
 * Output: true if the SSID is available, false otherwise
 */
boolean scanForSSID()
{
	//String ssid_verified = buildStringLoc(SSID_LENGTH, SSID_LOCATION); 
	String ssid_verified = buildStringLoc(SSID_STR);
	
	WiFi.mode(WIFI_STA);
	//Scan Wifi networks available
	int n = WiFi.scanNetworks();
	if (n == 0) return false;
	else 
	{
		for (int i = 0; i < n; i ++) 
		{
			if (strcmp(WiFi.SSID(i).c_str(), ssid_verified.c_str()) == 0) return true;
		}
	}
	return false;
}

static uint32_t nLastPublishTime = 0;


/*
 * setup
 * Desc: Called on startup, performs initializations
 * Input: void
 * Output: void
 */
void setup() 
{
	Serial.begin(115200);

	// Note: Namespace name is limited to 15 chars
	preferences.begin("my-app", false);
	//Get previous connection/configuration state from EEPROM
	configured = (int)preferences.getUInt(WIFI_CONNECT_BOOL, 0);
	
	// Get Interval Setting
	IntervalTime = (int)preferences.getUInt(INTERVAL, IntervalTime);
	
	//
	setupWifiServer();
	setupWiFi();

	// Firmware secure HTTP update from Bluemix Storage service, for testing

	// These variables are configured from bluemix service, we have to recieve them via mqtt message.
	X_AuthToken = "gAAAAABY9afVFcl5z3fgdD4tAu-pm8B2SnK8YJiCDknNcmxLZv0uO-9YAk8VXkRGMyI-Tms0-Gm2ohWSR3Hjr9N5GNHGTWBe3tKaHIONYiuO9atC7p8olJi4-q6nw6aHbAU_ACFYSBXcGnm_J-8Lc-VRWE3byxolJ4v4aUb9tIQMqok_6TCO1HI";
	//String url = "https://dal.objectstorage.open.softlayer.com/v1/AUTH_cc8f04a6048a4f68a20b9741928167b0/ESP32/SerialTest.bin";
	String url = "https://github.com/cf9234/testmike/raw/master/mikeesp.bin";

	{
		// check for : (http: or https:
		int index = url.indexOf(':');
		url.remove(0, (index + 3)); // remove http:// or https://
		index = url.indexOf('/');
		String host = url.substring(0, index);
		url.remove(0, index); // remove host part

		// get Authorization
		index = host.indexOf('@');
		if(index >= 0) {
			// auth info
			String auth = host.substring(0, index);
			host.remove(0, index + 1); // remove auth part including @
		}

		// get port
		index = host.indexOf(':');
		int _port;
		if(index >= 0) 
		{
			String _host = host.substring(0, index); // hostname
			host.remove(0, (index + 1)); // remove hostname + :
			_port = host.toInt(); // get port
		} 
		else 
		{
			_port = 443;
		}

		String _uri = url;
		// example syntax : t_httpUpdate_return ret = ESPhttpUpdate.update(host, _port, _uri, version, OTA_SERVER_ROOT_CA_PEM, reboot);
    //t_httpUpdate_return ret = ESPhttpUpdate.update(host, _port, _uri, "", OTA_SERVER_ROOT_CA_PEM, 1);
    t_httpUpdate_return ret = ESPhttpUpdate.update(host, 443, "/cf9234/testmike/raw/master/mikeesp.bin", "", OTA_SERVER_ROOT_CA_PEM, 1);
	}

	//setUpBluemix();
	//setupSensors();
	nLastPublishTime = millis();
}


/*
 * loop
 * Desc: Called immediately after setup(). Contains the logic to handle first connection, incorrect credentials, Wifi network
 *      down etc.
 * Input: void
 * Output: void
 */
void loop() 
{

	/*

	// Check if a client has connected
	WiFiClient client = serverWeb.available();
	if (client)
	{
		// Read the first line of the request
		String req = client.readStringUntil('\r');
		Serial.println("/n" + req);
		client.flush();

		String s = header;
		//Check to see if user submitted credentials, otherwise continue to serve up webpage
		if (req.indexOf("msg?ssid") != -1)
		{
			//Gather Wifi credentials
			Serial.println(F("Recieved network credentials. Waiting 10 seconds."));
			int s1 = req.indexOf("ssid=") + 5;
			int s2 = req.indexOf("&password");
			String ssid = req.substring(s1, s2);
			writeStringLoc(SSID_STR, decodeString(ssid));

			//Bluemix
			preferences.putUInt(CLOUD_CHOICE, 1);
			//EEPROM.write(CLOUD_CHOICE, byte(1));
			int p1 = s2 + 10;
			int p2 = p2 + req.indexOf("&orgB=");
			String password = req.substring(p1, p2);
			writeStringLoc(PASSWORD, decodeString(password));

			int st = req.indexOf("&orgB=") + 6;
			int fin = req.indexOf("&devtypeB=");
			String str_B = req.substring(st, fin);
			writeStringLoc(ORG, decodeString(str_B));

			st = fin + 10;
			fin = req.indexOf("&devidB=");
			str_B = req.substring(st, fin);
			writeStringLoc(DEVTYP, decodeString(str_B));

			st = fin + 8;
			fin = req.indexOf("&tokensB=");
			str_B = req.substring(st, fin);
			writeStringLoc(DEVTID, decodeString(str_B));

			st = fin + 9;
			fin = req.indexOf("&interval=");
			str_B = req.substring(st, fin);
			writeStringLoc(TOKEN, decodeString(str_B));

			st = fin + 10;
			fin = req.length() - String(" HTTP/1.1").length();
			str_B = req.substring(st, fin);
			IntervalTime = decodeString(str_B).toInt();
			preferences.putUInt(INTERVAL, IntervalTime);

			configured = 1;

			preferences.putUInt(WIFI_CONNECT_BOOL, 1);

			s = header;
			s += "<h1><center>Configuring ESP32 WATSON IOT Device</center></h1>";
			s += "</html>";
			client.print(s);
			strt = millis();
		}
		else
		{
			s +=
				"<body>"
				"<p>"
				"<center>"
				"<h1>EPS32 Device Configuration</h1>"
				"<div>"
				"</div>"
				"<form action='/msg'><p>SSID:  <input type='text' name='ssid' size=50 autofocus></p>"
				"<p>Password: <input type='password' name='password' size=50 autofocus></p>"
				"<div><p>Organization: <input type='text' name='orgB' size=50 autofocus></p><p>Device Type: <input type='text' name='devtypeB' size=50 autofocus></p>"
				"<p>Device Id: <input type='text' name='devidB' size=50 autofocus></p><p>Token: <input type='text' name='tokensB' size=50 autofocus></p></div>"
				"<p>Time inerval: <input type='text' name='interval' size=50 autofocus></p>"
				"<p><input type='submit' value='Submit'></p>"
				"</form>"
				"</center>";
			s += "</body></html>\n";
			// Send the response to the client
			client.print(s);
		}

		Serial.println(F("Client disconnected"));
	}

	if (nLastPublishTime + (IntervalTime * 1000) < millis())
	{
		nLastPublishTime = millis();

		if (WiFi.status() != WL_CONNECTED)
		{
			if (scanForSSID())
			{
				if (!setupWiFi()) //Wifi could not connect despite connecting previously
					return;
			}
			else
				return;
		}
		if (!setUpBluemix())
			Serial.println(F("I can't connect Mqtt, you have check setting or mqtt device!"));
		Serial.println("I will send data.");
		setupSensors();
		nLastPublishTime = millis();
	}
	*/
}

/*
 * getNetworkCredentials
 * Desc: Provides the UI that is served to the user to enter connection credentials and subsequently
 *      parses the HTTP response for the entered credentials.
 * Input: void
 * Output: void
 */
void getNetworkCredentials() 
{
	static int count = 0;
	count ++;
	if (count == 20)
	{
		Serial.println();
		count = 0;
	}
	
	if (strt && millis() - strt > waitInterval) 
	{
		//client.stop();
		ESP.restart();
	}
  
	Serial.print(".");
	
	// Check if a client has connected
	WiFiClient client = serverWeb.available();
	if (!client) 
	{
		return;
	}

	// Read the first line of the request
	String req = client.readStringUntil('\r');
	Serial.println("/n" + req);
	client.flush();

	String s = header;
	//Check to see if user submitted credentials, otherwise continue to serve up webpage
	if (req.indexOf("msg?ssid") != -1)
	{
		//Gather Wifi credentials
		Serial.println(F("Recieved network credentials. Waiting 10 seconds."));
		int s1 = req.indexOf("ssid=") + 5;
		int s2 = req.indexOf("&password");
		String ssid = req.substring(s1, s2);
		writeStringLoc(SSID_STR, decodeString(ssid));

		//Bluemix
		preferences.putUInt(CLOUD_CHOICE, 1);
		//EEPROM.write(CLOUD_CHOICE, byte(1));
		int p1 = s2 + 10;
		int p2 = p2 + req.indexOf("&orgB=");
		String password = req.substring(p1, p2);
		writeStringLoc(PASSWORD, decodeString(password));
    
		int st = req.indexOf("&orgB=") + 6;
		int fin = req.indexOf("&devtypeB=");
		String str_B = req.substring(st, fin);
		writeStringLoc(ORG, decodeString(str_B)); 

		st = fin + 10;
		fin = req.indexOf("&devidB=");
		str_B = req.substring(st, fin);
		writeStringLoc(DEVTYP, decodeString(str_B));

		st = fin + 8;
		fin = req.indexOf("&tokensB=");
		str_B = req.substring(st, fin);
		writeStringLoc(DEVTID, decodeString(str_B));

		st = fin + 9;
		fin = req.length() - String(" HTTP/1.1").length();
		str_B = req.substring(st, fin);
		writeStringLoc(TOKEN, decodeString(str_B));

		configured = 1;

		preferences.putUInt(WIFI_CONNECT_BOOL, 1);

		s = header;
		s += "<h1><center>Configuring ESP8266</center></h1>";
		s += "</html>"; 
		client.print(s);
		strt = millis();
	} 
	else 
	{
		s += 
		"<body>"    
		"<p>"
		"<center>"
		"<h1>EPS32 Device Configuration</h1>"
		"<div>"
		"</div>"
		"<form action='msg'><p>SSID:  <input type='text' name='ssid' size=50 autofocus></p>"
		"<p>Password: <input type='text' name='password' size=50 autofocus></p>"
		"<div><p>Organization: <input type='text' name='orgB' size=50 autofocus></p><p>Device Type: <input type='text' name='devtypeB' size=50 autofocus></p>"
		"<p>Device Id: <input type='text' name='devidB' size=50 autofocus></p><p>Token: <input type='text' name='tokensB' size=50 autofocus></p></div>"
		"<p><input type='submit' value='Submit'></p>"
		"</form>"
		"</center>";
		s += "</body></html>\n";
		// Send the response to the client
		client.print(s);
	}
    
	Serial.println(F("Client disconnected"));
}

/*
 * writeStringLoc
 * Desc: Writes a string to a specific location in EEPROM
 * Input: KeyStr is Key Name, str is contents
 * Output: void
 */
void writeStringLoc(char* KeyStr, String str) 
{
	preferences.putString(KeyStr, str);
	delay(250);
	Serial.print(F("Saved Key Name:"));
	Serial.print(KeyStr);
	Serial.print(F(", Value:"));
	Serial.println(str);
}

/*
 * buildStringLoc
 * Desc: Reads a string from Preferences
 * Input: String Key name
 * Output: String from Prefernces
 */
String buildStringLoc(char* KeyStr)
{
	return preferences.getString(KeyStr, "none");
}

/*
 * decodeString
 * Desc: For some reason certain Ascii characters get displayed differently after being submitted in a text field on a webpage.
 *       This function converts these values back to their standard format.
 * Input: Misconfigured Ascii value
 * Output: Correct string format
 */
String decodeString(String str) 
{
	str.replace("+", " ");
	str.replace("%21", "!");
	str.replace("%22", "");
	str.replace("%23", "#");
	str.replace("%24", "$");
	str.replace("%25", "%");
	str.replace("%26", "&");
	str.replace("%27", "'");
	str.replace("%28", "(");
	str.replace("%29", ")");
	str.replace("%2A", "*");
	str.replace("%2B", "+");
	str.replace("%2C", ",");
	str.replace("%2F", "/");
	str.replace("%3A", ":");
	str.replace("%3B", ";");
	str.replace("%3C", "<");
	str.replace("%3D", "=");
	str.replace("%3E", ">");
	str.replace("%3F", "?");
	str.replace("%40", "@");
	return str;
}

