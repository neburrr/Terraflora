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


function show_image(message) {
  var payload = message.payloadBytes
  var data = btoa(payload);
  console.log(data);
  const img = document.querySelector("img"); 
  img.src = "data:image/png;base64," + data;
}

function onConnect() {
  // Once a connection has been made, make a subscription and send a message.
  console.log("onConnect");
  client.subscribe("lettuceSample/realTimePub");
  client.subscribe("lettuceSample/clientState");
  client.subscribe("lettuceSample/log");
  client.subscribe("lettuceSample/systemState");

  

  document.getElementById("commsState").innerHTML = 'MQTT Communication Running  <span class="dotgreen"></span>';
 
  //message = new Paho.MQTT.Message("Hello");
  //message.destinationName = "/World";
  //client.send(message); 
};

function onMessageArrived(message) {
  console.log(JSON.parse(message.payloadString));
  console.log(message.destinationName);

  if(message.destinationName == "lettuceSample/realTimePub")
    processSensorMessage(JSON.parse(message.payloadString));
  if(message.destinationName == "lettuceSample/clientState")
    processClientMessage(JSON.parse(message.payloadString));
    if(message.destinationName == "lettuceSample/systemState")
    processStateMessage(JSON.parse(message.payloadString));
  if(message.destinationName == "lettuceSample/log")
    processLogMessage(JSON.parse(message.payloadString));


  //show_image(message)
};


function round(num){
  return (Math.round(num * 100) / 100).toFixed(2);
}

var greenhouseActive = false;
function greenHouseSetState(state){
  //console.log(state)
  if(state)
    document.getElementById("greenhouseState").innerHTML = 'Greenhouse Active   <span class="dotgreen"></span>';
  else
    document.getElementById("greenhouseState").innerHTML = 'Greenhouse Not Active   <span class="dotred"></span>';
};

function processLogMessage(message) {
  if(message.message == 'Restarting Greenhouse')
    greenHouseSetState(false);
  document.getElementById("logValues").innerHTML = message['message']
}

function processStateMessage(message) {
  document.getElementById("axisPos").innerHTML = 'X:' + message['SS']['X'] + ' - ' + 'Y:' + message['SS']['Y'] + ' - ' + 'Z:' + message['SS']['Z'] + ' (mm) ';

  if(message['SS']['L']){
    //document.getElementById("lightState").style.backgroundColor = "green";
    document.getElementById("lightState").innerHTML = "True"
  }
  else{
    //document.getElementById("lightState").style.backgroundColor = "red";
    document.getElementById("lightState").innerHTML = "False"
  }

  if(message['SS']['V']){ 
    //document.getElementById("ventState").style.backgroundColor = "green";
    document.getElementById("ventState").innerHTML = "True";
  }
  else{
    //document.getElementById("ventState").style.backgroundColor = "red";
    document.getElementById("ventState").innerHTML = "False";
  }

  if(message['SS']['W']){
    //document.getElementById("waterState").style.backgroundColor = "green";
    document.getElementById("waterState").innerHTML = "True";
  }
  else{
    //document.getElementById("waterState").style.backgroundColor = "red";
    document.getElementById("waterState").innerHTML = "False";
  }

  document.getElementById("setpointWater").innerHTML = message['SS']['WSP'] + ' %';
  if(message['SS']['M'])
    document.getElementById("greenhouseMode").innerHTML = "Manual";
  else
    document.getElementById("greenhouseMode").innerHTML = "PID Automatic";


};

function processClientMessage(message) {
  greenhouseActive = true;
  greenHouseSetState(greenhouseActive);

  document.getElementById("ipSoil").innerHTML = message['CS']['S']['IP'];
  if(message['CS']['S']['S'] )
    document.getElementById("stateSoil").innerHTML = 'Connected';
  else
    document.getElementById("stateSoil").innerHTML = 'Not Connected';

  document.getElementById("ipAxis").innerHTML = message['CS']['A']['IP'];
  if(message['CS']['A']['S'])
    document.getElementById("stateAxis").innerHTML = 'Connected';
  else
    document.getElementById("stateAxis").innerHTML = 'Not Connected';


  document.getElementById("ipCam").innerHTML = message['CS']['C']['IP'];
  if(message['CS']['C']['S'])
    document.getElementById("stateCam").innerHTML = 'Connected';
  else
    document.getElementById("stateCam").innerHTML = 'Not Connected';

    document.getElementById("stateAxis").innerHTML = 'Connected';
    document.getElementById("stateCam").innerHTML = 'Connected';
    document.getElementById("stateSoil").innerHTML = 'Connected';
};

function processSensorMessage(message) {
  
  greenhouseActive = true;
  greenHouseSetState(greenhouseActive);

  document.getElementById("tempInt").innerHTML = round(message['SD']['Int']['T']) + ' ºC';
  document.getElementById("arInt").innerHTML = round(message['SD']['Int']['AH']) + ' %';
  document.getElementById("soil").innerHTML = round(message['SD']['Int']['SH']) + ' %';
  document.getElementById("ldr").innerHTML = message['SD']['Int']['LDR'];
  document.getElementById("batVolt").innerHTML = round(message['SD']['Int']['VB']) + ' V';

  document.getElementById("tempExt").innerHTML = round(message['SD']['Ext']['T']) + ' ºC';
  document.getElementById("arExt").innerHTML = round(message['SD']['Ext']['AH']) + ' %';

  document.getElementById("waterLvl").innerHTML = round(message['SD']['Ext']['LVL']) + ' mL';
  
  a = [];
  alertList = [];
  if(message['SD']['Ext']['LVL'] < 1000)
    a.push("Reservoir almost empty!  (<1L%)");

  if(message['SD']['Int']['SH'] > 80)
    a.push("Soil Humidity too high!  (>80%)");
  if(message['SD']['Int']['SH'] < 30)
    a.push("Soil Humidity too low!  (<30%)");

  if(message['SD']['Int']['VB'] < 5.5)
    a.push("Battery Voltage too low!  (<5.5V%)");
  if(a.length == 0)
    a.push("No Alerts all good!");

  a.forEach(function(entry) {
      var singleObj = {}
      singleObj['Alert'] = entry;
      alertList.push(singleObj);
    });

  var myTable = createTable(alertList);
  document.getElementById('alertTable').appendChild(myTable);
  

};

/*
setTimeout(() => {
  greenhouseActive = false;
}, 10000);*/

//setInterval(greenHouseSetState(greenhouseActive), 5000);

/*
window.setInterval( function() {
  if(greenhouseActive)
    document.getElementById("greenhouseState").innerHTML = 'Greenhouse Active   <span class="dotgreen"></span>';
  else
    document.getElementById("greenhouseState").innerHTML = 'Greenhouse disabled   <span class="dotred"></span>';
}, 5000)*/

function createTable(list){
  tableHtml = '<thead><tr> <th>ALERTS</th> </tr></thead><tbody>'; // This empty string will hold the inner html of the table we just created

  for (var i = 0; i < list.length; ++i) {
      var alerts = list[i];

      // we can assign the id here instead of looping through the rows again
      tableHtml += '<tr id="row-' + i + '"><td>' + alerts.Alert + '</td></tr>';
  }  
  tableHtml += '</tbody>' 
  table.innerHTML = tableHtml; // We insert the html into the table element and parse it

  var rows = table.rows.length; // We already have a reference to the table, so no need of getElementById
  return table; // return the table. We still need to insert it into the dom
}

function triggerAlert(){
  alert('use :{"message": "water"} to water; use{"message": "moveAxisZ", "pos":"x"} to move Z AXIS a certain position; use{"message": "moveAxis", "pos":"G00 X- Y-"} to move XY AXIS a certain position; use {"message": "light"} to turn on lights; use {"message": "greenhouseScan"} to scan each plant}');

};

function changeMode(){
  console.log("changing mode")
  var mode;
  if (document.getElementById("greenhouseMode").innerHTML == "PID Automatic")
    mode = true;
  else  
    mode = false;

    client.publish(PUBLISHTOPIC, {
      "message": "mode",
      "value": mode
    });
}

function restart(){
  console.log("restart")
  client.publish(PUBLISHTOPIC, {
    "message": "restart",
  });
};

function waterPlant(){
  console.log("Watering")
  client.publish(PUBLISHTOPIC, {
    "message": "water",
  });
};

function sendCommand(text){
  console.log("publishing command")
  var $this = $(text);
  var val = $this.val();
  if(val == ''){
      console.log('no input');
  }else{
     console.log(val);
     client.publish(PUBLISHTOPIC, 
      JSON.parse(val)
    );
  }
};

function takephoto(){
  console.log("publishing scangreenhouse")
  client.publish(PUBLISHTOPIC, {
    "message": "greenhouseScan",
  });
};

function setWaterSetpoint(elem) {
  var $this = $(elem);
  var val = $this.val();
  if(val == ''){
      console.log('no input');
  }else{
     console.log(val);
     client.publish(PUBLISHTOPIC, {
      "message": "waterSetPoint",
      "value": val
    });
  }
}

//code starts here
var table = document.createElement('table');
table.classList.add('styled-table');
var myTable = createTable([{'Alert':'No Alerts all good!'}]);

document.getElementById('alertTable').appendChild(myTable);



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