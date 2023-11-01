#include <pgmspace.h>
 
#define SECRET
#define THINGNAME "LettuceCamera"
 
const char AWS_IOT_ENDPOINT[] = "a31prxj0vcgafr-ats.iot.us-west-2.amazonaws.com";
 
// Amazon Root CA 1
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";
 
// Device Certificate
static const char AWS_CERT_CRT[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
MIIDWTCCAkGgAwIBAgIUUcvdqSJHIlY23WL1ERUBUL7hbRswDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTIzMDYyNjE3MDAw
N1oXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMh/15ZhBgO9poDxpW1s
GysErbMzoE4AkYmjjFJ1mOUUuenYm47qjr+NFpGKtRjewWs8MmEr1NfHn7Yos015
txHJ4gF3TdDDE+raKwok/h4C+Rg5P/Pbmavo7nFFFqwytNqfTyO7xwVStFlY1Ve5
/mfCsWAeLw6I9hKy8KCl/jhN6XzwkkmI7XEcc/7q3QPGJRFuO3X0+p0vdMUzeWba
+T7LZAwVYn0UfbLnENosCZ6wARceHctXhEZDjbvnEGN67+4Vmuha6kcPwyCJIIQ0
3rY0fHDwqmkvga8Y+40Uwlelkce7Kt6ZL7aIDdIQokHC+/SGhl5cMm+g1xk3JXWa
cbMCAwEAAaNgMF4wHwYDVR0jBBgwFoAUV9qHSK35+JXIM1DE57XqdN1Nxn0wHQYD
VR0OBBYEFDMMhUi1TQPmGm135njow04RhPHyMAwGA1UdEwEB/wQCMAAwDgYDVR0P
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQAC8jndUY9T5zZAE9+QkF4jgIoz
u7piGHllwK5F9e864YyJ+R+k2LRVZrv3EY+I7f3Ke9R0C5SQgfVAG/YesADohaLT
PC1wPZA6MDuX8KOdBHrRWnogkx74Ckyil4XH4DXv9iFex374mFfA2IOUY1udihN7
dSgaBZVr0Gi+w+lj/SQpLsoeyJcUz5vzEPjBaP29rCTdwF8A0GBPiD93ma1EeJkW
+8getIvLGsJm6dM/EicrzoR+VS9MCs9NZdEzptD0cQLl5M62tFyH1unwNL30CssR
nNiIyje4J+rB6sLvYv6DY94B06MxxuK8zmNrckajaugrYBxHCwQdjNiyxWjI
-----END CERTIFICATE-----



)KEY";
 
// Device Private Key
static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
MIIEpQIBAAKCAQEAyH/XlmEGA72mgPGlbWwbKwStszOgTgCRiaOMUnWY5RS56dib
juqOv40WkYq1GN7BazwyYSvU18eftiizTXm3EcniAXdN0MMT6torCiT+HgL5GDk/
89uZq+jucUUWrDK02p9PI7vHBVK0WVjVV7n+Z8KxYB4vDoj2ErLwoKX+OE3pfPCS
SYjtcRxz/urdA8YlEW47dfT6nS90xTN5Ztr5PstkDBVifRR9sucQ2iwJnrABFx4d
y1eERkONu+cQY3rv7hWa6FrqRw/DIIkghDTetjR8cPCqaS+Brxj7jRTCV6WRx7sq
3pkvtogN0hCiQcL79IaGXlwyb6DXGTcldZpxswIDAQABAoIBAQCo7uaEIx3BOyM2
k+RRrPu7JEcCraYM8vetY+rQc5susWbv/H0dTs6V4Ne6K4fo+482vlKogxjj8qPE
BMkGp14zk8lkeNRM6IE4qTgIZSDjwpeE7H/RBR+WtD1rnM28q0Csz/wLXKUU/BlP
CsU/FEyhum5fUS9O7OGA4dpTUWlAoAzgNv1z7irfpEkmh83ujvSRmzJKhQtIfFlw
+z+jm8cFlXg8NNZVUrN8eT5xeyop0PH3DNszHk4r/aXjNjcah4ZI8g5FAe0PV50O
riIFeeIxX2ztTDSUTYHemJhF9gMxsCSkS+mykw0iFlWMrYkS2w/R0MtgN4bCUZbw
d0yxJCKJAoGBAPzBuGxCUB5qcYv9RaC7ul7KopCIdPkws6UROxq8a2sU/fV/k2+1
pNZSNv07sJT5E6n1VHMZzHUhXBaslPIqGmVcrMSVooU4uQ6ZNN+FptWTrGVVYWkM
tIqvP4HNof63XcfCe6i/Tnc7Bog4zOuUPNynG52Yti4DylBKDzILCJdXAoGBAMsS
djj+q4sfR6OuP0M730WrPymN8IF3IToqH0z+YSJZdYqw/3aWO7PmISaxdaX2Bxnk
2hT9ZFBlS6uEjsCAkODCiophDd2E0LwAKcUqlFxf5rdMsmSNKdixXiIU3xl4IC+o
9afNVOl/xztS67jKQfuLe5DacRnh5PBhwwqED0sFAoGARegWZ+rXWfI6wWslbNU/
MKR+3da/84Piy86+OCDQ3OcLbnEAiC8HDjfi+ZsetbVGGQW/e64w/FxjqenFsEyw
zEQbjKuOLaaImF8LXp8Ki5uyXLyGmpVnAcyClhYXreltSKijpwJyY0ux/M8o7icr
xiN54daw3H0z4Yz6Mu8WSzUCgYEAmYKdSDyMZSECQOPKAwNRpxLrLniUJOpvfc61
1rg9qX/C8VMX6wdqnnYK/XsNGqY/e/2aNl/O40+Pwlr0puUJoD6wCCeSGbD7MgNV
kCy99o6wlEQwXv6vcZWzOURZ2qEKg7zP50e7NsBXVas7cnfDJWqtwDTBZvVy4Zyl
ml92SBkCgYEA0XVlWMfCTUvxx/eb9K+bwHxlrYr1oGFm0Xcn1GW+UVIz1xsEI/ve
FQVdCAvBAEcMFmEbGQ1rGZl+s+yCBfku4DQt2qiWyN6PSoouWyaOOvqWXhXFOkFU
R7r5/yL1stPYmPFhPWAbQLqL+yuBcpvKWvDQZ/c+xYky446BrBxl3JA=
-----END RSA PRIVATE KEY-----



)KEY";