/// <reference types="aws-sdk" />

///const mqttClient = new AWSMqtt({
  //accessKeyId: "AKIAYVLFENIOAPZ5RVMW",
  //secretAccessKey: "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",
  //endpointAddress: "a31prxj0vcgafr-ats.iot.us-west-2.amazonaws.com",
  //region: "us-west-2"
//});

AWS.config.update({
  region: "us-west-2",
  // accessKeyId default can be used while using the downloadable version of DynamoDB. 
  // For security reasons, do not store AWS Credentials in your files. Use Amazon Cognito instead.
  accessKeyId: "AKIAYVLFENIOAPZ5RVMW",

  // secretAccessKey default can be used while using the downloadable version of DynamoDB. 
  // For security reasons, do not store AWS Credentials in your files. Use Amazon Cognito instead.
  secretAccessKey: "OYLoyZH4AxsLspNwCHOSppnoeHOy3NCIsXfUvJee"
});


var yLabelsState = {
  0 : 'False', 1 : 'True', 2 : '', 3 : '', 4 : ''
}

function getGraphTempOneDay(){
  const currentDate = new Date();
  const timestamp = currentDate.getTime();
  const oneDayTimestamp = 86400000; 
  let timestampValueToGraph = timestamp - (oneDayTimestamp);

  const dynamoClient = new AWS.DynamoDB();

  var params = {
          ExpressionAttributeValues: {
            ':device_id': {'S': 'greenhouse1'},
            ':sample_time': {'S': timestampValueToGraph.toString()}, //should be upload every time it gets called so the query does not need to handle a lot of elements
        },
          KeyConditionExpression: 'device_id = :device_id AND #dynobase_timestamp > :sample_time',
          ScanIndexForward: false,
          ExpressionAttributeNames: { "#dynobase_timestamp": "timestamp" },
          TableName: 'lettuceDBGraphs',
          Limit: 100
        };
  
  var dataOut;
  var stateVentilation = [];
  var timestampTemp = [];
  dynamoClient.query(params, function(err, data) {
    if (err) {
        console.log("Error", err);
    } else {
      dataOut = data.Items;

      console.log("Success: ", dataOut); 
      //console.log("Success: ",  Object.keys(dataOut).length);
      //console.log("Success: ", new Date(parseInt(dataOut[0].timestamp.S)));
      //console.log("Success: ", new Date(1695128400100).getHours() + ":" + new Date(1695128400100).getMinutes() + ", "+ new Date(1695128400100).toDateString());

      //console.log("ventilationState: ", data.Items[0].payload.M.ventilationState.BOOL);
      //console.log("waterState: ", data.Items[0].payload.M.waterState.BOOL);

      for (var i = 0; i < Object.keys(dataOut).length; i++) {
        stateVentilation[i] = data.Items[i].payload.M.ventilationState.BOOL;
        var a = data.Items[i].timestamp.S;
        timestampTemp[i] =  new Date(parseInt(a)).getUTCDate() + '/' + (new Date(parseInt(a)).getUTCMonth() + 1) + ', ' + (new Date(parseInt(a)).getUTCHours() + 1) + ':' + new Date(parseInt(a)).getUTCMinutes();
      }
      

      new Chart(
        document.getElementById('graphTemp'),{
        type: "line",
        responsive: true,
        maintainAspectRatio: false,
        data: {
          labels: timestampTemp,
          datasets: [{
            yAxisID: 'A',
            fill: false,
            lineTension: 0,
            label:"temperatureInt",
            backgroundColor: "rgba(0,0,255,1.0)",
            borderColor: "rgba(0,0,255,0.8)",
            data: dataOut.map(row =>row.payload.M.temperatureInt.N),
            pointRadius: 1,
          },
          {
            yAxisID: 'A',
            fill: false,
            lineTension: 0,
            label:"temperatureExt",
            backgroundColor: "rgba(0,255,0,1.0)",
            borderColor: "rgba(0,255,0,0.8)",
            data: dataOut.map(row =>row.payload.M.temperatureExt.N),
            pointRadius: 1,
          },
          {
            yAxisID: 'B',
            fill: false,
            lineTension: 0,
            label:"ventilationState",
            backgroundColor: "rgba(100,100,100,1.0)",
            borderColor: "rgba(100,100,100,0.8)",
            data: stateVentilation,
            pointRadius: 1,
          }],
        },
        options: {
          legend: {display: false},
          scales: {
            xAxes: [
              {
                display: true,
                scaleLabel: {
                  display: false,
                  labelString: "Date"
                },
                gridLines: {
                  display:false
              },
              ticks: {
                maxTicksLimit: 5,
                minRotation: 0,
                maxRotation: 0
              }
          }],
            yAxes: [
              {
                type: 'linear',
                display: true,
                position: 'left',
                id: 'A',
                ticks: {min: 0, max:40},
                scaleLabel: {
                  display: true,
                  labelString: "Temperature (°C)"
                },
                },
                {
                type: 'linear',
                display: true,
                position: 'right',
                id: 'B',
                ticks: {
                  min: -0.1, 
                  max:4,
                  stepSize: 1,
                  callback: function(value, index, values) {
                    return yLabelsState[value];
                }
                },
                scaleLabel: {
                  display: true,
                  labelString: "Ventilation State"
                },
                gridLines: {
                display:false
            }
            },
            
            ],
          }
        }
      });
      }
    });
}

function getGraphHumOneDay(){
  const currentDate = new Date();
  const timestamp = currentDate.getTime();
  const oneDayTimestamp = 86400000; 
  let timestampValueToGraph = timestamp - (oneDayTimestamp);

  const dynamoClient = new AWS.DynamoDB();

  var params = {
          ExpressionAttributeValues: {
            ':device_id': {'S': 'greenhouse1'},
            ':sample_time': {'S': timestampValueToGraph.toString()}, //should be upload every time it gets called so the query does not need to handle a lot of elements
        },
          KeyConditionExpression: 'device_id = :device_id AND #dynobase_timestamp > :sample_time',
          ScanIndexForward: false,
          ExpressionAttributeNames: { "#dynobase_timestamp": "timestamp" },
          TableName: 'lettuceDBGraphs',
          Limit: 100
        };
  
  var dataOut;
  var stateWater = [];
  var timestampHum = [];
  dynamoClient.query(params, function(err, data) {
    if (err) {
        console.log("Error", err);
    } else {
      dataOut = data.Items;

      for (var i = 0; i < Object.keys(dataOut).length; i++) {
        stateWater[i] = data.Items[i].payload.M.waterState.BOOL;
        var a = data.Items[i].timestamp.S;
        timestampHum[i] =  new Date(parseInt(a)).getUTCDate() + '/' + (new Date(parseInt(a)).getUTCMonth() + 1) + ', ' + (new Date(parseInt(a)).getUTCHours() + 1) + ':' + new Date(parseInt(a)).getUTCMinutes();
      }

      new Chart(
        document.getElementById('graphHum'),{
        type: "line",
        xValueType: "dateTime",
        responsive: true,
        maintainAspectRatio: false,
        data: {
          labels: timestampHum,
          datasets: [{
            yAxisID: 'A',
            fill: false,
            label: 'airHumidityInt',
            lineTension: 0,
            backgroundColor: "rgba(0,0,255,1.0)",
            borderColor: "rgba(0,0,255,0.8)",
            data: dataOut.map(row =>row.payload.M.airHumidityInt.N),
            pointRadius: 1,
            
          },
          {
            yAxisID: 'A',
            fill: false,
            label: 'airHumidityExt',
            lineTension: 0,
            backgroundColor: "rgba(0,255,0,1.0)",
            borderColor: "rgba(0,255,0,0.8)",
            data: dataOut.map(row =>row.payload.M.airHumidityExt.N),
            pointRadius: 1,
          },
          {
            yAxisID: 'A',
            fill: false,
            label: 'soilHumidity',
            lineTension: 0,
            backgroundColor: "rgba(255,0,0,1.0)",
            borderColor: "rgba(255,0,0,0.8)",
            data: dataOut.map(row =>row.payload.M.soilHumidity.N),
            pointRadius: 1,
          },
          {
            yAxisID: 'B',
            fill: false,
            label: 'waterState',
            lineTension: 0,
            backgroundColor: "rgba(100,100,100,1.0)",
            borderColor: "rgba(100,100,100,0.8)",
            data: stateWater,
            pointRadius: 1,
          }],
        },
        options: {
          legend: {display: false},
          scales: {
            xAxes: [
              {
                
                
                display: true,
                scaleLabel: {
                  display: false,
                  labelString: "Date"
                },
                gridLines: {
                  display:false
              },
              ticks: {
                maxTicksLimit: 5,
                minRotation: 0,
                maxRotation: 0
              }
              }
            ],
            yAxes: [
              {
                type: 'linear',
                display: true,
                position: 'left',
                id: 'A',
                ticks: {min: 30, max:100},
                scaleLabel: {
                  display: true,
                  labelString: "Humidity (%)"
                },
                },
                {
                type: 'linear',
                display: true,
                position: 'right',
                id: 'B',
                ticks: {
                  min: -0.1, 
                  max:4,
                  stepSize: 1,
                  callback: function(value, index, values) {
                    return yLabelsState[value];
                },
              },
                scaleLabel: {
                  display: true,
                  labelString: "Water State"
                
                },
                gridLines: {
                display:false
            }
            },
            
            ],
          }
        }
      });
      }
    });
}

function getGraphTempOneWeek(){
  const currentDate = new Date();
  const timestamp = currentDate.getTime();
  const oneDayTimestamp = 86400000; 
  let timestampValueToGraph = timestamp - 9*(oneDayTimestamp);

  const dynamoClient = new AWS.DynamoDB();

  var params = {
          ExpressionAttributeValues: {
            ':device_id': {'S': 'greenhouse1'},
            ':sample_time': {'S': timestampValueToGraph.toString()}, //should be upload every time it gets called so the query does not need to handle a lot of elements
        },
          KeyConditionExpression: 'device_id = :device_id AND #dynobase_timestamp > :sample_time',
          ScanIndexForward: false,
          ExpressionAttributeNames: { "#dynobase_timestamp": "timestamp" },
          TableName: 'lettuceDBGraphs',
          Limit: 1000
        };
  
  var dataOut;
  var stateVentilation = [];
  var timestampTemp = [];
  dynamoClient.query(params, function(err, data) {
    if (err) {
        console.log("Error", err);
    } else {
      dataOut = data.Items;

      //console.log("Success: ", dataOut[0].timestamp.S); 
      //console.log("Success: ",  Object.keys(dataOut).length);
      //console.log("Success: ", new Date(parseInt(dataOut[0].timestamp.S)));
      //console.log("Success: ", new Date(1695128400100).getHours() + ":" + new Date(1695128400100).getMinutes() + ", "+ new Date(1695128400100).toDateString());

      //console.log("ventilationState: ", data.Items[0].payload.M.ventilationState.BOOL);
      //console.log("waterState: ", data.Items[0].payload.M.waterState.BOOL);

      for (var i = 0; i < Object.keys(dataOut).length; i++) {
        stateVentilation[i] = data.Items[i].payload.M.ventilationState.BOOL;
        var a = data.Items[i].timestamp.S;
        timestampTemp[i] =  new Date(parseInt(a)).getUTCDate() + '/' + (new Date(parseInt(a)).getUTCMonth() + 1) + ', ' + (new Date(parseInt(a)).getUTCHours() + 1) + ':' + new Date(parseInt(a)).getUTCMinutes();
      }
      

      new Chart(
        document.getElementById('graphTemp'),{
        type: "line",
        responsive: true,
        maintainAspectRatio: false,
        data: {
          labels: timestampTemp,
          datasets: [{
            yAxisID: 'A',
            fill: false,
            lineTension: 0,
            label:"temperatureInt",
            backgroundColor: "rgba(0,0,255,1.0)",
            borderColor: "rgba(0,0,255,0.8)",
            data: dataOut.map(row =>row.payload.M.temperatureInt.N),
            pointRadius: 1,
          },
          {
            yAxisID: 'A',
            fill: false,
            lineTension: 0,
            label:"temperatureExt",
            backgroundColor: "rgba(0,255,0,1.0)",
            borderColor: "rgba(0,255,0,0.8)",
            data: dataOut.map(row =>row.payload.M.temperatureExt.N),
            pointRadius: 1,
          },
          {
            yAxisID: 'B',
            fill: false,
            lineTension: 0,
            label:"ventilationState",
            backgroundColor: "rgba(100,100,100,1.0)",
            borderColor: "rgba(100,100,100,0.8)",
            data: stateVentilation,
            pointRadius: 1,
          }],
        },
        options: {
          legend: {display: false},
          scales: {
            xAxes: [
              {
                display: true,
                scaleLabel: {
                  display: false,
                  labelString: "Date"
                },
                gridLines: {
                  display:false
              },
              ticks: {
                maxTicksLimit: 5,
                minRotation: 0,
                maxRotation: 0
              }
          }],
            yAxes: [
              {
                type: 'linear',
                display: true,
                position: 'left',
                id: 'A',
                ticks: {min: 0, max:40},
                scaleLabel: {
                  display: true,
                  labelString: "Temperature (°C)"
                },
                },
                {
                type: 'linear',
                display: true,
                position: 'right',
                id: 'B',
                ticks: {
                  min: -0.1, 
                  max:4,
                  stepSize: 1,
                  callback: function(value, index, values) {
                    return yLabelsState[value];
                }
                },
                scaleLabel: {
                  display: true,
                  labelString: "Ventilation State"
                },
                gridLines: {
                display:false
            }
            },
            
            ],
          }
        }
      });
      }
    });
}

function getGraphHumOneWeek(){
  const currentDate = new Date();
  const timestamp = currentDate.getTime();
  const oneDayTimestamp = 86400000; 
  let timestampValueToGraph = timestamp - 9*(oneDayTimestamp);

  const dynamoClient = new AWS.DynamoDB();

  var params = {
          ExpressionAttributeValues: {
            ':device_id': {'S': 'greenhouse1'},
            ':sample_time': {'S': timestampValueToGraph.toString()}, //should be upload every time it gets called so the query does not need to handle a lot of elements
        },
          KeyConditionExpression: 'device_id = :device_id AND #dynobase_timestamp > :sample_time',
          ScanIndexForward: false,
          ExpressionAttributeNames: { "#dynobase_timestamp": "timestamp" },
          TableName: 'lettuceDBGraphs',
          Limit: 1000
        };
  
  var dataOut;
  var stateWater = [];
  var timestampHum = [];
  dynamoClient.query(params, function(err, data) {
    if (err) {
        console.log("Error", err);
    } else {
      dataOut = data.Items;

      for (var i = 0; i < Object.keys(dataOut).length; i++) {
        stateWater[i] = data.Items[i].payload.M.waterState.BOOL;
        var a = data.Items[i].timestamp.S;
        timestampHum[i] =  new Date(parseInt(a)).getUTCDate() + '/' + (new Date(parseInt(a)).getUTCMonth() + 1) + ', ' + (new Date(parseInt(a)).getUTCHours() + 1) + ':' + new Date(parseInt(a)).getUTCMinutes();
      }

      new Chart(
        document.getElementById('graphHum'),{
        type: "line",
        xValueType: "dateTime",
        responsive: true,
        maintainAspectRatio: false,
        data: {
          labels: timestampHum,
          datasets: [{
            yAxisID: 'A',
            fill: false,
            label: 'airHumidityInt',
            lineTension: 0,
            backgroundColor: "rgba(0,0,255,1.0)",
            borderColor: "rgba(0,0,255,0.8)",
            data: dataOut.map(row =>row.payload.M.airHumidityInt.N),
            pointRadius: 1,
            
          },
          {
            yAxisID: 'A',
            fill: false,
            label: 'airHumidityExt',
            lineTension: 0,
            backgroundColor: "rgba(0,255,0,1.0)",
            borderColor: "rgba(0,255,0,0.8)",
            data: dataOut.map(row =>row.payload.M.airHumidityExt.N),
            pointRadius: 1,
          },
          {
            yAxisID: 'A',
            fill: false,
            label: 'soilHumidity',
            lineTension: 0,
            backgroundColor: "rgba(255,0,0,1.0)",
            borderColor: "rgba(255,0,0,0.8)",
            data: dataOut.map(row =>row.payload.M.soilHumidity.N),
            pointRadius: 1,
          },
          {
            yAxisID: 'B',
            fill: false,
            label: 'waterState',
            lineTension: 0,
            backgroundColor: "rgba(100,100,100,1.0)",
            borderColor: "rgba(100,100,100,0.8)",
            data: stateWater,
            pointRadius: 1,
          }],
        },
        options: {
          legend: {display: false},
          scales: {
            xAxes: [
              {
                
                
                display: true,
                scaleLabel: {
                  display: false,
                  labelString: "Date"
                },
                gridLines: {
                  display:false
              },
              ticks: {
                maxTicksLimit: 5,
                minRotation: 0,
                maxRotation: 0
              }
              }
            ],
            yAxes: [
              {
                type: 'linear',
                display: true,
                position: 'left',
                id: 'A',
                ticks: {min: 30, max:100},
                scaleLabel: {
                  display: true,
                  labelString: "Humidity (%)"
                },
                },
                {
                type: 'linear',
                display: true,
                position: 'right',
                id: 'B',
                ticks: {
                  min: -0.1, 
                  max:4,
                  stepSize: 1,
                  callback: function(value, index, values) {
                    return yLabelsState[value];
                },
              },
                scaleLabel: {
                  display: true,
                  labelString: "Water State"
                
                },
                gridLines: {
                display:false
            }
            },
            
            ],
          }
        }
      });
      }
    });
}

function getGraphTempOneMonth(){
  const currentDate = new Date();
  const timestamp = currentDate.getTime();
  const oneDayTimestamp = 86400000; 
  let timestampValueToGraph = timestamp - 31*(oneDayTimestamp);

  const dynamoClient = new AWS.DynamoDB();

  var params = {
          ExpressionAttributeValues: {
            ':device_id': {'S': 'greenhouse1'},
            ':sample_time': {'S': timestampValueToGraph.toString()}, //should be upload every time it gets called so the query does not need to handle a lot of elements
        },
          KeyConditionExpression: 'device_id = :device_id AND #dynobase_timestamp > :sample_time',
          ScanIndexForward: false,
          ExpressionAttributeNames: { "#dynobase_timestamp": "timestamp" },
          TableName: 'lettuceDBGraphs',
          Limit: 10000
        };
  
  var dataOut;
  var stateVentilation = [];
  var timestampTemp = [];
  dynamoClient.query(params, function(err, data) {
    if (err) {
        console.log("Error", err);
    } else {
      dataOut = data.Items;

      //console.log("Success: ", dataOut[0].timestamp.S); 
      //console.log("Success: ",  Object.keys(dataOut).length);
      //console.log("Success: ", new Date(parseInt(dataOut[0].timestamp.S)));
      //console.log("Success: ", new Date(1695128400100).getHours() + ":" + new Date(1695128400100).getMinutes() + ", "+ new Date(1695128400100).toDateString());

      //console.log("ventilationState: ", data.Items[0].payload.M.ventilationState.BOOL);
      //console.log("waterState: ", data.Items[0].payload.M.waterState.BOOL);

      for (var i = 0; i < Object.keys(dataOut).length; i++) {
        stateVentilation[i] = data.Items[i].payload.M.ventilationState.BOOL;
        var a = data.Items[i].timestamp.S;
        timestampTemp[i] =  new Date(parseInt(a)).getUTCDate() + '/' + (new Date(parseInt(a)).getUTCMonth() + 1) + ', ' + (new Date(parseInt(a)).getUTCHours() + 1) + ':' + new Date(parseInt(a)).getUTCMinutes();
      }
      

      new Chart(
        document.getElementById('graphTemp'),{
        type: "line",
        responsive: true,
        maintainAspectRatio: false,
        data: {
          labels: timestampTemp,
          datasets: [{
            yAxisID: 'A',
            fill: false,
            lineTension: 0,
            label:"temperatureInt",
            backgroundColor: "rgba(0,0,255,1.0)",
            borderColor: "rgba(0,0,255,0.8)",
            data: dataOut.map(row =>row.payload.M.temperatureInt.N),
            pointRadius: 1,
          },
          {
            yAxisID: 'A',
            fill: false,
            lineTension: 0,
            label:"temperatureExt",
            backgroundColor: "rgba(0,255,0,1.0)",
            borderColor: "rgba(0,255,0,0.8)",
            data: dataOut.map(row =>row.payload.M.temperatureExt.N),
            pointRadius: 1,
          },
          {
            yAxisID: 'B',
            fill: false,
            lineTension: 0,
            label:"ventilationState",
            backgroundColor: "rgba(100,100,100,1.0)",
            borderColor: "rgba(100,100,100,0.8)",
            data: stateVentilation,
            pointRadius: 1,
          }],
        },
        options: {
          legend: {display: false},
          scales: {
            xAxes: [
              {
                display: true,
                scaleLabel: {
                  display: false,
                  labelString: "Date"
                },
                gridLines: {
                  display:false
              },
              ticks: {
                maxTicksLimit: 5,
                minRotation: 0,
                maxRotation: 0
              }
          }],
            yAxes: [
              {
                type: 'linear',
                display: true,
                position: 'left',
                id: 'A',
                ticks: {min: 0, max:40},
                scaleLabel: {
                  display: true,
                  labelString: "Temperature (°C)"
                },
                },
                {
                type: 'linear',
                display: true,
                position: 'right',
                id: 'B',
                ticks: {
                  min: -0.1, 
                  max:4,
                  stepSize: 1,
                  callback: function(value, index, values) {
                    return yLabelsState[value];
                }
                },
                scaleLabel: {
                  display: true,
                  labelString: "Ventilation State"
                },
                gridLines: {
                display:false
            }
            },
            
            ],
          }
        }
      });
      }
    });
}

function getGraphHumOneMonth(){
  const currentDate = new Date();
  const timestamp = currentDate.getTime();
  const oneDayTimestamp = 86400000; 
  let timestampValueToGraph = timestamp - 31*(oneDayTimestamp);

  const dynamoClient = new AWS.DynamoDB();

  var params = {
          ExpressionAttributeValues: {
            ':device_id': {'S': 'greenhouse1'},
            ':sample_time': {'S': timestampValueToGraph.toString()}, //should be upload every time it gets called so the query does not need to handle a lot of elements
        },
          KeyConditionExpression: 'device_id = :device_id AND #dynobase_timestamp > :sample_time',
          ScanIndexForward: false,
          ExpressionAttributeNames: { "#dynobase_timestamp": "timestamp" },
          TableName: 'lettuceDBGraphs',
          Limit: 10000
        };
  
  var dataOut;
  var stateWater = [];
  var timestampHum = [];
  dynamoClient.query(params, function(err, data) {
    if (err) {
        console.log("Error", err);
    } else {
      dataOut = data.Items;

      for (var i = 0; i < Object.keys(dataOut).length; i++) {
        stateWater[i] = data.Items[i].payload.M.waterState.BOOL;
        var a = data.Items[i].timestamp.S;
        timestampHum[i] =  new Date(parseInt(a)).getUTCDate() + '/' + (new Date(parseInt(a)).getUTCMonth() + 1) + ', ' + (new Date(parseInt(a)).getUTCHours() + 1) + ':' + new Date(parseInt(a)).getUTCMinutes();
      }

      new Chart(
        document.getElementById('graphHum'),{
        type: "line",
        xValueType: "dateTime",
        responsive: true,
        maintainAspectRatio: false,
        data: {
          labels: timestampHum,
          datasets: [{
            yAxisID: 'A',
            fill: false,
            label: 'airHumidityInt',
            lineTension: 0,
            backgroundColor: "rgba(0,0,255,1.0)",
            borderColor: "rgba(0,0,255,0.8)",
            data: dataOut.map(row =>row.payload.M.airHumidityInt.N),
            pointRadius: 1,
            
          },
          {
            yAxisID: 'A',
            fill: false,
            label: 'airHumidityExt',
            lineTension: 0,
            backgroundColor: "rgba(0,255,0,1.0)",
            borderColor: "rgba(0,255,0,0.8)",
            data: dataOut.map(row =>row.payload.M.airHumidityExt.N),
            pointRadius: 1,
          },
          {
            yAxisID: 'A',
            fill: false,
            label: 'soilHumidity',
            lineTension: 0,
            backgroundColor: "rgba(255,0,0,1.0)",
            borderColor: "rgba(255,0,0,0.8)",
            data: dataOut.map(row =>row.payload.M.soilHumidity.N),
            pointRadius: 1,
          },
          {
            yAxisID: 'B',
            fill: false,
            label: 'waterState',
            lineTension: 0,
            backgroundColor: "rgba(100,100,100,1.0)",
            borderColor: "rgba(100,100,100,0.8)",
            data: stateWater,
            pointRadius: 1,
          }],
        },
        options: {
          legend: {display: false},
          scales: {
            xAxes: [
              {
                
                
                display: true,
                scaleLabel: {
                  display: false,
                  labelString: "Date"
                },
                gridLines: {
                  display:false
              },
              ticks: {
                maxTicksLimit: 5,
                minRotation: 0,
                maxRotation: 0
              }
              }
            ],
            yAxes: [
              {
                type: 'linear',
                display: true,
                position: 'left',
                id: 'A',
                ticks: {min: 30, max:100},
                scaleLabel: {
                  display: true,
                  labelString: "Humidity (%)"
                },
                },
                {
                type: 'linear',
                display: true,
                position: 'right',
                id: 'B',
                ticks: {
                  min: -0.1, 
                  max:4,
                  stepSize: 1,
                  callback: function(value, index, values) {
                    return yLabelsState[value];
                },
              },
                scaleLabel: {
                  display: true,
                  labelString: "Water State"
                
                },
                gridLines: {
                display:false
            }
            },
            
            ],
          }
        }
      });
      }
    });
}

getGraphTempOneDay();
getGraphHumOneDay();

function aDayGraphs(){
  getGraphTempOneDay();
  getGraphHumOneDay();
}

function aWeekGraphs(){
  getGraphTempOneWeek();
  getGraphHumOneWeek();
}

function aMonthGraphs(){
  getGraphTempOneMonth();
  getGraphHumOneMonth();
}


