const REGION = "us-west-2";   
const IOT_ENDPOINT = "a31prxj0vcgafr-ats.iot.us-west-2.amazonaws.com";

//const KEY_ID = "AKIAYVLFENIOAPZ5RVMW";
//const SECRET_KEY = "OYLoyZH4AxsLspNwCHOSppnoeHOy3NCIsXfUvJee";
const KEY_ID = "AKIAYVLFENIOOSHLN5EB"; 
const SECRET_KEY = "xs5DaIT1qwcCNGx3L9+kQ09DMDDYOl/AXoi+VTdp"; 
const PUBLISHTOPIC = "lettuce/order"

function p4(){} 
p4.sign = function(key, msg) { 
    const hash = CryptoJS.HmacSHA256(msg, key); 
    return hash.toString(CryptoJS.enc.Hex); 
}; 
p4.sha256 = function(msg) { 
    const hash = CryptoJS.SHA256(msg); 
    return hash.toString(CryptoJS.enc.Hex); 
}; 
p4.getSignatureKey = function(key, dateStamp, regionName, serviceName) { 
    const kDate = CryptoJS.HmacSHA256(dateStamp, 'AWS4' + key); 
    const kRegion = CryptoJS.HmacSHA256(regionName, kDate); 
    const kService = CryptoJS.HmacSHA256(serviceName, kRegion); 
    const kSigning = CryptoJS.HmacSHA256('aws4_request', kService); 
    return kSigning; 
};  

function getEndpoint() { 
    // date & time 
    const dt = (new Date()).toISOString().replace(/[^0-9]/g, ""); 
    const ymd = dt.slice(0,8); 
    const fdt = `${ymd}T${dt.slice(8,14)}Z` 
      
    const scope = `${ymd}/${REGION}/iotdevicegateway/aws4_request`; 
    const ks = encodeURIComponent(`${KEY_ID}/${scope}`); 
    let qs = `X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=${ks}&X-Amz-Date=${fdt}&X-Amz-SignedHeaders=host`; 
    const req = `GET\n/mqtt\n${qs}\nhost:${IOT_ENDPOINT}\n\nhost\n${p4.sha256('')}`; 
    qs += '&X-Amz-Signature=' + p4.sign( 
        p4.getSignatureKey( SECRET_KEY, ymd, REGION, 'iotdevicegateway'), 
        `AWS4-HMAC-SHA256\n${fdt}\n${scope}\n${p4.sha256(req)}`
    ); 
    return `wss://${IOT_ENDPOINT}/mqtt?${qs}`; 
} 
   
function onConnectionLost(responseObject) {
  if (responseObject.errorCode !== 0)
	console.log("onConnectionLost:"+responseObject.errorMessage);
  document.getElementById("commsState").innerHTML = 'MQTT Communication not Running  <span class="dotred"></span>';
};

function onMessageArrived(message) {
  console.log(JSON.parse(message.payloadString));
  greenhouseActive = true;
  greenHouseSetState(greenhouseActive);

  
  if(message.destinationName == "lettuceSample/log")
    processMessage(JSON.parse(message.payloadString));
  //show_image(message)
};

function show_image(message) {
  var payload = message.payloadBytes
  var data = btoa(payload);
  console.log(data);
  const img = document.querySelector("img"); 
  img.src = "data:image/png;base64," + data;
}

function onConnect() {
  console.log("onConnect");
  client.subscribe("lettuceSample/log");

  document.getElementById("commsState").innerHTML = 'MQTT Communication Running  <span class="dotgreen"></span>';

};

var greenhouseActive = false;
function greenHouseSetState(state){
  //console.log(state)
  if(state)
    document.getElementById("greenhouseState").innerHTML = 'Greenhouse active   <span class="dotgreen"></span>';
  else
    document.getElementById("greenhouseState").innerHTML = 'Greenhouse not active   <span class="dotred"></span>';
};


function processMessage(message) {

};

function sendCommand(){
  console.log("publishing")
  client.publish(PUBLISHTOPIC, {
    "message": "takephoto",
  });
};

const clientId = Math.random().toString(36).substring(7); 
const client = new Paho.MQTT.Client(getEndpoint(), clientId);
client.publish = function(topic, payload) { 
          let payloadText = JSON.stringify(payload); 
          let message = new Paho.MQTT.Message(payloadText); 
          message.destinationName = topic; 
          message.qos = 0; 
          client.send(message); 
      } 
client.onConnectionLost = onConnectionLost;
client.onMessageArrived = onMessageArrived;

client.connect({onSuccess:onConnect,
  onFailure: function(e) { 
            console.log("failed connect")
            console.log(e); 
            document.getElementById("commsState").innerHTML = 'Failed to connect to MQTT Broker  <span class="dotred"></span>';
        },
  
  useSSL: true, 
  timeout:  20, 
  mqttVersion: 3, 
  keepAliveInterval:30,});